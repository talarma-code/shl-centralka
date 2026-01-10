#include  "HaCommunicationTask.h"
#include  "MqttConfiguration.h"

#define MODEM_RX 13  // ESP32 RX -> TX modemu
#define MODEM_TX 14  // ESP32 TX -> RX modemu
#define HARDWARE_MODEM_NUMBER 2  

#define MAX_GPRS_CONNECT_ATTEMPTS 5
#define MAX_MQTT_CONNECT_ATTEMPTS 5 
#define MAX_SOFTWARE_MODEM_RESETS 3
#define MAX_WAIT_FOR_NETWORK_ATTEMPTS 12

#define TIME_5_MINUTEs 300000
#define TIME_15_MINUTEs 900000
#define TIME_30_MINUTEs 1800000



HaCommunicationTask::HaCommunicationTask() : ActiveTask("HaCommunicationTask", 8192, 3, 1),
    haQueue(10), 
    timer(1, 1000, SystemTimerT<SystemMessagePacket, TimerToSystemMessage>::Mode::OneShot, ActiveQueueRef<SystemMessagePacket>(haQueue.nativeHandle()), TimerToSystemMessage()),
    hardwareModem (HARDWARE_MODEM_NUMBER), 
    tinyGsmModem(hardwareModem), 
    tinyGsmClient(tinyGsmModem),
    mqttClient(tinyGsmClient)
{
    _waitForNetworkCounter = 0;
    _gprsConnectCounter = 0;
    _mqttConnectCounter = 0;
    _softwareModemResetCounter = 0;
    _hardwerModemReserCounter = 0;
}

void HaCommunicationTask::setup() {
    connectionManager(ModemState::ModemPowerOn);
}

void HaCommunicationTask::connectionManager(ModemState s) {
    switch (s) {
        case ModemState::ModemPowerOn:
            Serial.println("Powering on modem...");
            modemPowerOn();
            clearSoftwareResetCounters();
            _state = ModemState::InitSerial;
            timer.start(1500);
            break;

        case ModemState::InitSerial:
            hardwareModem.begin(57600, SERIAL_8N1, MODEM_RX, MODEM_TX);
            Serial.println("Serial started");
            _state = ModemState::RestartModem;
             timer.start(150);
            break;

        case ModemState::RestartModem:
            Serial.println("Initializing modem (restart)...");
            tinyGsmModem.restart();
            _state = ModemState::WaitForNetwork;
            _softwareModemResetCounter++;
            if (_softwareModemResetCounter >= MAX_SOFTWARE_MODEM_RESETS) {
                Serial.println("Too many software resets, performing hardware reset...");
                _state = ModemState::Error;
            }
            timer.start(2000);
            break;

        case ModemState::WaitForNetwork:
            Serial.println("Waiting for network (poll)...");
            if (tinyGsmModem.isNetworkConnected()) {
                Serial.println("Network OK");
                _state = ModemState::GprsConnect;
                timer.start(100);
            } else {
                if (_waitForNetworkCounter >= MAX_WAIT_FOR_NETWORK_ATTEMPTS) {
                    _state = ModemState::RestartModem;
                }
                _waitForNetworkCounter++;
                timer.start(500);
            }
            break;

        case ModemState::GprsConnect:
            Serial.print("Connecting to APN: ");
            Serial.println(_apn);
            if (tinyGsmModem.gprsConnect(_apn)) {
                Serial.println("GPRS OK");
                _state = ModemState::MqttConnect;
                timer.start(100);
            } else {
                Serial.println("GPRS connect failed, retrying...");
                _gprsConnectCounter++;
                if (_gprsConnectCounter >= MAX_GPRS_CONNECT_ATTEMPTS) {
                    _state = ModemState::RestartModem;
                }
                timer.start(2000);
            }
            break;

        case ModemState::MqttConnect:
            Serial.println("Attempting MQTT connect...");
            mqttClient.setServer(MQTT_HOST, MQTT_PORT);
            if (mqttClient.connect("SIM7000Client01", MQTT_USER, MQTT_PASS)) {
                Serial.println("MQTT connected");
                Serial.println("schedule modem running");
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
            break;

        case ModemState::Running:
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
            break;

        case ModemState::Error:
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
            break;

        case ModemState::ModemPowerOff:
            Serial.println("Powering off modem...");
            modemPowerOff();
            _state = ModemState::ModemPowerOn;
            timer.start(3000);
            break;

        default:
            break;
    }
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
    _mqttConnectCounter = 0;
    _gprsConnectCounter = 0;   
    _softwareModemResetCounter = 0; 
}



