#pragma once
// Use SIM7000 modem types explicitly (avoid depending on global macro ordering)
#include <TinyGsmClientSIM7000.h>
#include <PubSubClient.h>
#include "ResetSim7000Modem.h"
#include "WaitForNetworkMonitor.h"
#include "MqttMeasurementsPublisher.h"
#include "HaRunningMonitor.h"
#include "ActiveTask.h"
#include "ActiveQueue.h"
#include "IntertaskDataModel.h"
#include "SystemTimer.h"
#include "WaitForSIM7000Init.h"



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
    HaCommunicationTask(QueueHandle_t mainTaskQueueHandle);
    void setup() override;
    void loop() override;
    QueueHandle_t getQueueHandle() const {
        return haQueue.nativeHandle();
    }
private: 
    enum class ModemState {
        ModemPowerOn,
        InitSerial,
        WaitForSIM7000Init,
        SoftwareRestartModem,
        WaitForNetwork,
        GprsConnect,
        MqttConnect,
        PublishResetReason,
        Running,
        Error
    };

    ActiveQueue<SystemMessagePacket> haQueue;
    ActiveQueueRef<ApplicationMessagePacket> mainTaskQueue;
    SystemTimerT<SystemMessagePacket, TimerToSystemMessage> timer;
    HardwareSerial hardwareModem;
    TinyGsmSim7000 tinyGsmModem;
    TinyGsmSim7000::GsmClientSim7000 tinyGsmClient;
    PubSubClient mqttClient;
    ResetSim7000Modem resetter;
    WaitForSIM7000Init waitForSIM7000Init;
    WaitForNetworkMonitor waitForNetworkMonitor;
    MqttMeasurementsPublisher measPublisher{mqttClient, "lacko/shl_c1/telemetry"};
    MqttMeasurementsPublisher historyPublisher{mqttClient, "lacko/shl_c1/history"};
    MqttMeasurementsPublisher statusPublisher{mqttClient, "lacko/shl_c1/status"};
    HaRunningMonitor runningMonitor{mqttClient, statusPublisher};

    ModemState _state = ModemState::ModemPowerOn;
    uint8_t _errorGprsConnectCounter = 0;
    uint8_t _errorMqttConnectCounter = 0;
    uint8_t _errorModemSoftwareResetCounter = 0;
    uint8_t _hardwerModemReserCounter = 0;


    //const char* _apn = "internet";        // APN for Orange Poland/nju (SIM7000G), no username/password
    const char* _apn = "www.mobilny.pl";    // APN for T-Mobile Poland/otvarta (SIM7000E), no username/password

    

     void connectionManager(ModemState s);
     void modemPowerOff();
     void modemPowerOn();
     void clearSoftwareErrorCounters();
     void findReasonAndReconnect();

     // --- Handlers for each ModemState ---
    void handleModemPowerOn();
    void handleInitSerial();
    void handleWaitForSIM7000Init();
    void handleSoftwareRestartModem();
    void handleWaitForNetwork();
    void handleGprsConnect();
    void handleMqttConnect();
    void publishResetReason();
    void handleRunning();
    void handleError();

    uint32_t syncNetworkTime();
};
