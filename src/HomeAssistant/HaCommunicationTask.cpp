#include  "HaCommunicationTask.h"
#include  "MqttConfiguration.h"

#define MODEM_RX 13  // ESP32 RX -> TX modemu
#define MODEM_TX 14  // ESP32 TX -> RX modemu
#define HARDWARE_MODEM_NUMBER 2  


HaCommunicationTask::HaCommunicationTask() : ActiveTask("HaCommunicationTask", 8192, 3, 1),
    haQueue(10), 
    // construct timer to send SystemMessagePacket using TimerToSystemMessage converter
    timer(1, 1000, SystemTimerT<SystemMessagePacket, TimerToSystemMessage>::Mode::OneShot, ActiveQueueRef<SystemMessagePacket>(haQueue.nativeHandle()), TimerToSystemMessage()),
    hardwareModem (HARDWARE_MODEM_NUMBER), 
    tinyGsmModem(hardwareModem), 
    tinyGsmClient(tinyGsmModem),
    mqttClient(tinyGsmClient)
{
}

void HaCommunicationTask::setup() {
    // Kick off the state machine
    connectionManager(ModemState::InitSerial);
}

void HaCommunicationTask::connectionManager(ModemState s) {
    switch (s) {
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
            timer.start(3000);
            break;

        case ModemState::WaitForNetwork:
            Serial.println("Waiting for network (poll)...");
            if (tinyGsmModem.isNetworkConnected()) {
                Serial.println("Network OK");
                _state = ModemState::GprsConnect;
                timer.start(100);
            } else {
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
                timer.start(100);
            } else {
                Serial.print("MQTT connect failed, rc=");
                Serial.println(mqttClient.state());
                timer.start(2000);
            }
            break;

        case ModemState::Running:
            // In Running state: process MQTT internals and attempt reconnect if needed.
            if (mqttClient.connected()) {
                mqttClient.loop();
                timer.start(2000);           // schedule next MQTT refresh
            } else {
                Serial.println("MQTT disconnected, reconnecting...");
                if (!tinyGsmModem.isNetworkConnected()) {
                    _state = ModemState::WaitForNetwork;
                    timer.start(500);
                }
                else if (!tinyGsmModem.isGprsConnected()) {
                    _state = ModemState::GprsConnect;
                    timer.start(500);
                }
                else {
                    _state = ModemState::MqttConnect;
                    timer.start(200);
                }
            }
            break;

        case ModemState::Error:
            _state = ModemState::Error;
            Serial.println("Modem state ERROR");
            // Could implement backoff or reset
            timer.start(5000);
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




