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

void HaCommunicationTask::connectionManager(ModemState s) {
    switch (s) {
        case ModemState::InitSerial:
            // Start UART
            hardwareModem.begin(57600, SERIAL_8N1, MODEM_RX, MODEM_TX);
            Serial.println("Serial started");
            // give UART some time to settle, next state will be RestartModem
            timer.start(100);
            _state = ModemState::RestartModem;
            break;

        case ModemState::RestartModem:
            _state = ModemState::RestartModem;
            Serial.println("Initializing modem (restart)...");
            tinyGsmModem.restart();
            // wait for modem to settle and then poll network
            timer.start(3000);
            break;

        case ModemState::WaitForNetwork:
            _state = ModemState::WaitForNetwork;
            Serial.println("Waiting for network (poll)...");
            // perform immediate check; if not connected start poll timer
            if (tinyGsmModem.isNetworkConnected()) {
                Serial.println("Network OK");
                connectionManager(ModemState::GprsConnect);
            } else {
                timer.start(500);
            }
            break;

        case ModemState::GprsConnect:
            _state = ModemState::GprsConnect;
            Serial.print("Connecting to APN: ");
            Serial.println(_apn);
            if (tinyGsmModem.gprsConnect(_apn)) {
                Serial.println("GPRS OK");
                // proceed to MQTT connect
                connectionManager(ModemState::MqttConnect);
            } else {
                Serial.println("GPRS connect failed, retrying...");
                timer.start(2000);
                // remain in GprsConnect until next timer
            }
            break;

        case ModemState::MqttConnect:
            _state = ModemState::MqttConnect;
            Serial.println("Attempting MQTT connect...");
            mqttClient.setServer(MQTT_HOST, MQTT_PORT);
            if (mqttClient.connect("SIM7000Client01", MQTT_USER, MQTT_PASS)) {
                Serial.println("MQTT connected");
                connectionManager(ModemState::Running);

            } else {
                Serial.print("MQTT connect failed, rc=");
                Serial.println(mqttClient.state());
                timer.start(2000);
                // remain in MqttConnect until timer triggers retry
            }
            break;

        case ModemState::Running:
            _state = ModemState::Running;
            Serial.println("Modem running");
            // In Running state: process MQTT internals and attempt reconnect if needed.
            if (mqttClient.connected()) {
                mqttClient.loop();
            } else {
                Serial.println("MQTT disconnected, attempting reconnect...");
                if (mqttClient.connect("SIM7000Client01", MQTT_USER, MQTT_PASS)) {
                    Serial.println("MQTT reconnected");
                    _lastSend = millis();
                } else {
                    Serial.print("MQTT reconnect failed, rc=");
                    Serial.println(mqttClient.state());
                }
            }
            // schedule next MQTT refresh
            timer.start(2000);
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

void HaCommunicationTask::setup() {
    // Kick off the state machine
    connectionManager(ModemState::InitSerial);
}

void HaCommunicationTask::loop()
{
    // Process incoming system messages (timer events)
    SystemMessagePacket msg;
    // non-blocking receive
    if (haQueue.receive(msg, 0)) {
        if (msg.type == SystemDataType::Timer) {
            // timer fired
            connectionManager(_state);
        }
    }

    // Periodic publish when running (MQTT loop/reconnect handled by timer-driven connectionManager)
    if (_state == ModemState::Running) {
        if (mqttClient.connected() && (millis() - _lastSend > 10000)) {
            _lastSend = millis();
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
}




