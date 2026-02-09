#include  "HaCommunicationTask.h"
#include  "MqttConfiguration.h"
#include  "CommunicationParams.h"
#include  "Log.h"

#define MODEM_RX 13  // ESP32 RX -> TX modemu
#define MODEM_TX 14  // ESP32 TX -> RX modemu
#define HARDWARE_MODEM_NUMBER 2  

HaCommunicationTask::HaCommunicationTask() : ActiveTask("HaCommunicationTask", 10240, 3, 1),
    haQueue(10), 
    timer(1, 1000, SystemTimerT<SystemMessagePacket, TimerToSystemMessage>::Mode::OneShot, ActiveQueueRef<SystemMessagePacket>(haQueue.nativeHandle()), TimerToSystemMessage()),
    hardwareModem (HARDWARE_MODEM_NUMBER), 
    tinyGsmModem(hardwareModem), 
    tinyGsmClient(tinyGsmModem),
    mqttClient(tinyGsmClient),
    resetter(tinyGsmModem),
    waitForNetworkMonitor(tinyGsmModem)
{
    _errorGprsConnectCounter = 0;
    _errorMqttConnectCounter = 0;
    _errorModemSoftwareResetCounter = 0;
    _hardwerModemReserCounter = 0;

    mqttClient.setKeepAlive(60);        // seconds
    mqttClient.setSocketTimeout(30);    // seconds
    mqttClient.setBufferSize(2048);     // większy bufor na ramki MQTT (temat + payload)

    // Configure running monitor: 500 ms loop, check connection every 3 loops, heartbeat every 60 s
    runningMonitor.configure(500, 3, 60);
}

void HaCommunicationTask::setup() {
    printf("HaCommunicationTask setup\r\n");
    connectionManager(ModemState::ModemPowerOn);
}

void HaCommunicationTask::loop()
{
    SystemMessagePacket msg;
    if (haQueue.receive(msg)) {
        if (msg.type == SystemDataType::Timer) {
            connectionManager(_state);
        }
        else if (msg.type == SystemDataType::Measurements) {
            if (_state != ModemState::Running) {
                LOG_INFO("Cannot publish, modem not running yeat");
            } else if (!mqttClient.connected()) {
                LOG_ERROR("MQTT disconnected, Measurements");
                findReasonAndReconnect();
            } else if (!measPublisher.publishPacket(msg.payload.measurementData)) {
                LOG_ERROR("Publish measurement packet failed - reconnecting...");
                findReasonAndReconnect();
            }
        }
    }
    resetWatchdog();
}

void HaCommunicationTask::connectionManager(ModemState s) {
    switch (s) {
        case ModemState::ModemPowerOn:
            handleModemPowerOn();
            break;
        case ModemState::InitSerial:
            handleInitSerial();
            break;
        case ModemState::SoftwareRestartModem:
            handleSoftwareRestartModem();
            break;
        case ModemState::WaitForNetwork:
            handleWaitForNetwork();
            break;
        case ModemState::GprsConnect:
            handleGprsConnect();
            break;
        case ModemState::MqttConnect:
            handleMqttConnect();
            break;
        case ModemState::Running:
            handleRunning();
            break;
        case ModemState::Error:
            handleError();
            break;
        case ModemState::ModemPowerOff:
            handleModemPowerOff();
            break;
        default:
            break;
    }
}

// --- Dedicated handlers for each ModemState ---
void HaCommunicationTask::handleModemPowerOn() {
    LOG_INFO("Modem power ON");
    modemPowerOn();
    clearSoftwareErrorCounters();
    _state = ModemState::InitSerial;
    timer.start(1500);
}

void HaCommunicationTask::handleInitSerial() {
    hardwareModem.begin(57600, SERIAL_8N1, MODEM_RX, MODEM_TX);
    LOG_INFO("Serial started");
    _state = ModemState::SoftwareRestartModem;
    timer.start(150);
}

void HaCommunicationTask::handleSoftwareRestartModem() {
    LOG_INFO("Restarting modem...");
    const auto res = resetter.step();
    switch (res.next) {
        case ResetSim7000Modem::Next::Stay:
            _state = ModemState::SoftwareRestartModem;
            break;
        case ResetSim7000Modem::Next::WaitForNetwork:
            _state = ModemState::WaitForNetwork;
            break;
        case ResetSim7000Modem::Next::Error:
            _state = ModemState::Error;
            if (res.failed) {
                _errorModemSoftwareResetCounter++;
            }
            break;
    }
    timer.start(res.delayMs);
}

void HaCommunicationTask::handleWaitForNetwork() {
    LOG_INFO("Waiting for network (poll)...");
    const auto res = waitForNetworkMonitor.step();
    switch (res.next) {
        case WaitForNetworkMonitor::Next::Stay:
            _state = ModemState::WaitForNetwork;
            break;
        case WaitForNetworkMonitor::Next::GprsConnect:
            LOG_INFO("Network OK");
            _state = ModemState::GprsConnect;
            break;
        case WaitForNetworkMonitor::Next::SoftwareRestartModem:
            _state = ModemState::SoftwareRestartModem;
            break;
    }
    timer.start(res.delayMs);
}

void HaCommunicationTask::handleGprsConnect() {
    LOG_INFO("Connecting to APN: %s", _apn);
    if (tinyGsmModem.gprsConnect(_apn)) {
        LOG_INFO("GPRS OK");
        _state = ModemState::MqttConnect;
        timer.start(100);
    } else {
        LOG_INFO("GPRS connect failed, retrying...");
        _errorGprsConnectCounter++;
        if (_errorGprsConnectCounter >= MAX_GPRS_CONNECT_ATTEMPTS) {
            _state = ModemState::SoftwareRestartModem;
        }
        timer.start(2000);
    }
}

void HaCommunicationTask::handleMqttConnect() {
    LOG_INFO("Attempting MQTT connect...");
    mqttClient.setServer(MQTT_HOST, MQTT_PORT);
    // Tune MQTT session for cellular link stability
    //todo: use setSocketTimeout() to set timeout for mqtt connect but only for broker, if gprs lost, will be waiting longer
    if (mqttClient.connect("SIM7000Client01", MQTT_USER, MQTT_PASS)) {
        LOG_INFO("MQTT connected");
        _state = ModemState::Running;
        clearSoftwareErrorCounters();
        timer.start(100);
    } else {
        LOG_INFO("MQTT connect failed, rc=%d", mqttClient.state());
        _errorMqttConnectCounter++;
        if (_errorMqttConnectCounter >= MAX_MQTT_CONNECT_ATTEMPTS) {
            _state = ModemState::GprsConnect;
        }
        timer.start(500);
    }
}

void HaCommunicationTask::handleRunning() {
    _hardwerModemReserCounter = 0;

    const auto res = runningMonitor.step();
    if (res.next == HaRunningMonitor::Next::ReconnectNeeded) {
        LOG_ERROR("MQTT disconnected, state:handleRunning rc=%d", mqttClient.state());
        findReasonAndReconnect();
        return; // timer is started inside findReasonAndReconnect()
    }

    // Stay in Running state and re-arm timer according to monitor suggestion
    _state = ModemState::Running;
    timer.start(res.delayMs);
}

void HaCommunicationTask::findReasonAndReconnect() {
    if (!tinyGsmModem.isNetworkConnected()) {
        _state = ModemState::WaitForNetwork;
        timer.start(200);
    }
    else if (!tinyGsmModem.isGprsConnected()) {
        _state = ModemState::GprsConnect;
        timer.start(200);
    }
    else {
        _state = ModemState::MqttConnect;
        timer.start(200);
    }
}

void HaCommunicationTask::handleError() {
    if (_hardwerModemReserCounter == 0)
    {
        _state = ModemState::ModemPowerOff;
        timer.start(200);
    }
    if (_hardwerModemReserCounter == 1)
    {
        _state = ModemState::ModemPowerOff;
        timer.start(TIME_5_MINUTEs);
    }
    if (_hardwerModemReserCounter == 2)
    {
        _state = ModemState::ModemPowerOff;
        timer.start(TIME_15_MINUTEs);
    }
    if (_hardwerModemReserCounter >= 3 && _hardwerModemReserCounter < 8)
    {
        _state = ModemState::ModemPowerOff;
        timer.start(TIME_30_MINUTEs);
    }
    if (_hardwerModemReserCounter >= 8)
    {
        _state = ModemState::ModemPowerOff;
        timer.start(TIME_1_HOUR);
    }

    LOG_ERROR("Modem state ERROR: hardware reset: %d", _hardwerModemReserCounter);
    clearSoftwareErrorCounters();
}

void HaCommunicationTask::handleModemPowerOff() {
    LOG_INFO("Powering off modem...");
    _hardwerModemReserCounter++;
    modemPowerOff();
    _state = ModemState::ModemPowerOn;
    timer.start(3000);
}

void HaCommunicationTask::modemPowerOff() {
    //TODO: when hardware ready - enable modem 
}

void HaCommunicationTask::modemPowerOn() {
    //TODO: when hardware ready - disable modem
}

void HaCommunicationTask::clearSoftwareErrorCounters() {
    _errorGprsConnectCounter = 0;
    _errorMqttConnectCounter = 0;
    _errorModemSoftwareResetCounter = 0;
}

//todo: issueses
// 1. changne reset modem logic - use AT command instead of library 
// 2. the same for gprs connect 
// 3. add subscribe to mqtt topic and process incoming messages - ping each 60s to check mqtt connection
//     3.1 when connect to mqtt failed , check ping. 