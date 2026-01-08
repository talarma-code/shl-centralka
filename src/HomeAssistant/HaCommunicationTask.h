#pragma once
// Use SIM7000 modem types explicitly (avoid depending on global macro ordering)
#include <TinyGsmClientSIM7000.h>
#include <PubSubClient.h>

#include "ActiveTask.h"
#include "ActiveQueue.h"
#include "IntertaskDataModel.h"
#include "SystemTimer.h"
#include "MqttConfiguration.h"


// Converter: convert TimerEvent -> SystemMessagePacket (ISR-safe, no allocation)
struct TimerToSystemMessage {
    SystemMessagePacket operator()(const TimerEvent& t) const noexcept {
        SystemMessagePacket m{};
        m.type = SystemDataType::Timer;
        m.payload.timerData.timerId = t.timerId;
        return m;
    }
};

class HaCommunicationTask : public ActiveTask {
public:
    HaCommunicationTask();
    void setup() override;
    void loop() override;
private: 
    enum class ModemState {
        Idle,
        InitSerial,
        RestartModem,
        WaitForNetwork,
        GprsConnect,
        MqttConnect,
        Running,
        Error
    };

    void connectionManager(ModemState s);
    // Handle timer events delivered as SystemMessagePacket (extracted from loop)

    ActiveQueue<SystemMessagePacket> haQueue;
    // Use templated SystemTimer to send SystemMessagePacket via converter
    SystemTimerT<SystemMessagePacket, TimerToSystemMessage> timer;
    HardwareSerial hardwareModem;
    TinyGsmSim7000 tinyGsmModem;
    TinyGsmSim7000::GsmClientSim7000 tinyGsmClient;
    PubSubClient mqttClient;

    ModemState _state = ModemState::Idle;
    unsigned long _lastSend = 0;

    // config
    const char* _apn = "internet";

};
