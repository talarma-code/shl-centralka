#pragma once
// Use SIM7000 modem types explicitly (avoid depending on global macro ordering)
#include <TinyGsmClientSIM7000.h>
#include <PubSubClient.h>
#include "ResetSim7000Modem.h"

#include "ActiveTask.h"
#include "ActiveQueue.h"
#include "IntertaskDataModel.h"
#include "SystemTimer.h"



// Converter: convert TimerEvent -> SystemMessagePacket (ISR-safe, no allocation)
class TimerToSystemMessage {
    public:
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
        ModemPowerOn,
        InitSerial,
        SoftwareRestartModem,
        WaitForNetwork,
        // GprsStartAttach,
        // GprsWaitAttach,
        CheckNetwork,
        GprsConnect,
        MqttConnect,
        Running,
        Error,
        ModemPowerOff
    };

    ActiveQueue<SystemMessagePacket> haQueue;
    SystemTimerT<SystemMessagePacket, TimerToSystemMessage> timer;
    HardwareSerial hardwareModem;
    TinyGsmSim7000 tinyGsmModem;
    TinyGsmSim7000::GsmClientSim7000 tinyGsmClient;
    PubSubClient mqttClient;
    ResetSim7000Modem resetter;

    ModemState _state = ModemState::Idle;
    uint8_t _waitForNetworkCounter;
    uint8_t _checkNetworkCounter;

    uint8_t _waitGprsConnectCounter;
    uint8_t _errorGprsConnectCounter;

    uint8_t _mqttConnectCounter;
    uint8_t _errorModemSoftwareResetCounter;
    uint8_t _hardwerModemReserCounter;


    const char* _apn = "internet";

     void connectionManager(ModemState s);
     void modemPowerOff();
     void modemPowerOn();
     void clearSoftwareResetCounters();

     // --- Handlers for each ModemState ---
     void handleModemPowerOn();
     void handleInitSerial();
        void handleSoftwareRestartModem();
     void handleWaitForNetwork();
     void handleGprsConnect();
     void handleMqttConnect();
     void handleRunning();
     void handleError();
     void handleModemPowerOff();




};
