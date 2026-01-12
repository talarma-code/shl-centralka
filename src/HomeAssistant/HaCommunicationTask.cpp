#include  "HaCommunicationTask.h"
#include  "MqttConfiguration.h"
#include  "CommunicationParams.h"

#define MODEM_RX 13  // ESP32 RX -> TX modemu
#define MODEM_TX 14  // ESP32 TX -> RX modemu
#define HARDWARE_MODEM_NUMBER 2  

HaCommunicationTask::HaCommunicationTask() : ActiveTask("HaCommunicationTask", 8192, 3, 1),
    haQueue(10), 
    timer(1, 1000, SystemTimerT<SystemMessagePacket, TimerToSystemMessage>::Mode::OneShot, ActiveQueueRef<SystemMessagePacket>(haQueue.nativeHandle()), TimerToSystemMessage()),
    hardwareModem (HARDWARE_MODEM_NUMBER), 
    tinyGsmModem(hardwareModem), 
    tinyGsmClient(tinyGsmModem),
    mqttClient(tinyGsmClient)
{
    _waitForNetworkCounter = 0;
    _waitGprsConnectCounter = 0;
    _errorGprsConnectCounter = 0;
    _mqttConnectCounter = 0;
    _checkNetworkCounter = 0;
    _errorModemSoftwareResetCounter = 0;
    _hardwerModemReserCounter = 0;
    _modemRestartPhase = RestartPhase::Begin;
    _modemRestartAttempts = 0;
}

void HaCommunicationTask::setup() {
    printf("HaCommunicationTask setup\r\n");
    connectionManager(ModemState::ModemPowerOn);
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
        // case ModemState::GprsStartAttach:
        // case ModemState::GprsWaitAttach:
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
    Serial.println("Powering on modem...");
    modemPowerOn();
    clearSoftwareResetCounters();
    _state = ModemState::InitSerial;
    timer.start(1500);
}

void HaCommunicationTask::handleInitSerial() {
    hardwareModem.begin(57600, SERIAL_8N1, MODEM_RX, MODEM_TX);
    Serial.println("Serial started");
    _state = ModemState::SoftwareRestartModem;
    timer.start(150);
}

void HaCommunicationTask::handleSoftwareRestartModem() {
    switch (_modemRestartPhase) {
        case RestartPhase::Begin: {
            Serial.println("[Restart] Begin");
            _modemRestartAttempts = 0;
            _modemRestartPhase = RestartPhase::SendCfun0;
            timer.start(100);
            break;
        }
        case RestartPhase::SendCfun0: {
            Serial.println("[Restart] RF OFF (+CFUN=0)");
            tinyGsmModem.sendAT("+CFUN=0");
            _modemRestartPhase = RestartPhase::WaitCfun0;
            timer.start(RESTART_DELAY_SEND_CFUN_MS);
            break;
        }
        case RestartPhase::WaitCfun0: {
            tinyGsmModem.sendAT("+CFUN?");
            int r = tinyGsmModem.waitResponse(RESTART_DELAY_WAIT_CFUN0_MS, "+CFUN: 0");
            if (r == 1) {
                Serial.println("[Restart] RF is OFF");
                _modemRestartPhase = RestartPhase::SendCfun1;
                timer.start(200);
            } else {
                _modemRestartAttempts++;
                if (_modemRestartAttempts >= MODEM_RESTART_RETRY_LIMIT) {
                    Serial.println("[Restart] CFUN=0 failed, aborting");
                    _errorModemSoftwareResetCounter++;
                    _state = ModemState::Error;
                    timer.start(200);
                } else {
                    timer.start(RESTART_DELAY_WAIT_CFUN0_MS);
                }
            }
            break;
        }
        case RestartPhase::SendCfun1: {
            Serial.println("[Restart] RF ON (+CFUN=1)");
            tinyGsmModem.sendAT("+CFUN=1");
            _modemRestartPhase = RestartPhase::WaitCfun1;
            timer.start(RESTART_DELAY_SEND_CFUN_MS);
            break;
        }
        case RestartPhase::WaitCfun1: {
            tinyGsmModem.sendAT("+CFUN?");
            int r = tinyGsmModem.waitResponse(RESTART_DELAY_WAIT_CFUN1_MS, "+CFUN: 1");
            if (r == 1) {
                Serial.println("[Restart] RF is ON");
                _modemRestartPhase = RestartPhase::ProbeAT;
                timer.start(300);
            } else {
                _modemRestartAttempts++;
                if (_modemRestartAttempts >= MODEM_RESTART_RETRY_LIMIT) {
                    Serial.println("[Restart] CFUN=1 failed, aborting");
                    _errorModemSoftwareResetCounter++;
                    _state = ModemState::Error;
                    timer.start(200);
                } else {
                    timer.start(RESTART_DELAY_WAIT_CFUN1_MS);
                }
            }
            break;
        }
        case RestartPhase::ProbeAT: {
            // Check if modem responds to AT, indicating it's alive
            tinyGsmModem.sendAT("AT");
            int r = tinyGsmModem.waitResponse(RESTART_DELAY_PROBE_AT_MS);
            if (r == 1) {
                Serial.println("[Restart] Modem ready");
                _modemRestartPhase = RestartPhase::Done;
                // Transition to WaitForNetwork like original flow
                _state = ModemState::WaitForNetwork;
                timer.start(2000);
            } else {
                _modemRestartAttempts++;
                if (_modemRestartAttempts >= MODEM_RESTART_RETRY_LIMIT) {
                    Serial.println("[Restart] Modem not responding to AT");
                    _errorModemSoftwareResetCounter++;
                    _state = ModemState::Error;
                    timer.start(200);
                } else {
                    timer.start(RESTART_DELAY_PROBE_AT_MS);
                }
            }
            break;
        }
        case RestartPhase::Done: {
            // Reset local phase to allow next restart cycle if needed
            _modemRestartPhase = RestartPhase::Begin;
            _modemRestartAttempts = 0;
            break;
        }
    }
}

void HaCommunicationTask::handleWaitForNetwork() {
    Serial.println("Waiting for network (poll)...");
    if (tinyGsmModem.isNetworkConnected()) {
        Serial.println("Network OK");
        _state = ModemState::GprsConnect;
        timer.start(100);
    } else {
        if (_waitForNetworkCounter >= MAX_WAIT_FOR_NETWORK_ATTEMPTS) {
            _waitForNetworkCounter = 0;
            _state = ModemState::SoftwareRestartModem;
        }
        _waitForNetworkCounter++;
        timer.start(500);
    }
}

void HaCommunicationTask::handleGprsConnect() {
    Serial.print("Connecting to APN: ");
    Serial.println(_apn);
    if (tinyGsmModem.gprsConnect(_apn)) {
        Serial.println("GPRS OK");
        _state = ModemState::MqttConnect;
        timer.start(100);
    } else {
        Serial.println("GPRS connect failed, retrying...");
        _errorGprsConnectCounter++;
        if (_errorGprsConnectCounter >= MAX_GPRS_CONNECT_ATTEMPTS) {
            _state = ModemState::SoftwareRestartModem;
        }
        timer.start(2000);
    }
}

void HaCommunicationTask::handleMqttConnect() {
    Serial.println("Attempting MQTT connect...");
    mqttClient.setServer(MQTT_HOST, MQTT_PORT);
    //todo: use setSocketTimeout() to set timeout for mqtt connect but only for broker, if gprs lost, will be waiting longer
    if (mqttClient.connect("SIM7000Client01", MQTT_USER, MQTT_PASS)) {
        Serial.println("MQTT connected");
        //mqttClient.subscribe("test/#");
        _state = ModemState::Running;
        clearSoftwareResetCounters();
        timer.start(100);
    } else {
        Serial.print("MQTT connect failed, rc=");
        Serial.println(mqttClient.state());
        _mqttConnectCounter++;
        if (_mqttConnectCounter >= MAX_MQTT_CONNECT_ATTEMPTS) {
            _state = ModemState::GprsConnect;
        }
        timer.start(500);
    }
}

void HaCommunicationTask::handleRunning() {
    // In Running state: process MQTT internals and attempt reconnect if needed.
    if (mqttClient.connected()) {
        _hardwerModemReserCounter = 0;
        mqttClient.loop();
        timer.start(1000);           // schedule next MQTT refresh
    } else {
        Serial.println("MQTT disconnected, reconnecting...");
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
    if (_hardwerModemReserCounter >= 3)
    {
        _state = ModemState::ModemPowerOff;
        timer.start(TIME_30_MINUTEs);
    }
    Serial.println("Modem state ERROR: hardware reset: ");
    Serial.println(_hardwerModemReserCounter);
    clearSoftwareResetCounters();
}

void HaCommunicationTask::handleModemPowerOff() {
    Serial.println("Powering off modem...");
    modemPowerOff();
    _state = ModemState::ModemPowerOn;
    timer.start(3000);
}

void HaCommunicationTask::loop()
{
    SystemMessagePacket msg;
    if (haQueue.receive(msg)) {
        if (msg.type == SystemDataType::Timer) {
            connectionManager(_state);
        }
        else if (msg.type == SystemDataType::Measurements) {
            if (_state == ModemState::Running) {
                if (mqttClient.connected()) {
                    float temp = 23.5;
                    char payload[16];
                    dtostrf(temp, 4, 2, payload);
                    if (!mqttClient.publish("test/sim7000/temperatura", payload)) {
                        Serial.println("Publish failed");
                    } else {
                        Serial.print("Sending MQTT: ");
                        Serial.println(payload);
                    }
                }   
            }
            else {
                Serial.println("Cannot publish, modem not running, state:");
                Serial.println((int)_state);
            }
        }
    }
}

void HaCommunicationTask::modemPowerOff() {
    //TODO: when hardware ready - enable modem 
}

void HaCommunicationTask::modemPowerOn() {
    //TODO: when hardware ready - disable modem
}

void HaCommunicationTask::clearSoftwareResetCounters() {
    _waitForNetworkCounter = 0;
    _waitGprsConnectCounter = 0;
     _errorGprsConnectCounter = 0;
    _mqttConnectCounter = 0;
    _checkNetworkCounter = 0;
    _errorModemSoftwareResetCounter = 0;
}

//todo: issueses
// 1. changne reset modem logic - use AT command instead of library 
// 2. the same for gprs connect 
// 3. add subscribe to mqtt topic and process incoming messages - ping each 60s to check mqtt connection
//     3.1 when connect to mqtt failed , check ping. 