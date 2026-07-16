#pragma once
#include "ShlProtocolTransport.h"
#include "IShlProtocolReceiver.h"
#include "ActiveTask.h"
#include "ActivePoolRef.h"
#include "SystemTimer.h"
#include "IntertaskDataModel.h"
#include "RtcDs3231.h"
#include "HeaterFsm.h"
#include "PowerMeterFsm.h"
#include "HourlySurplusAlgorithm.h"

class TimerToApplicationMessage {
    public:
    ApplicationMessagePacket operator()(const TimerEvent& t) const noexcept {
        ApplicationMessagePacket m{};
        m.type = ApplicationCommandType::Timer;
        m.payload.timerEvent.timerId = t.timerId;
        return m;
    }
};

class ApplicationTask : public IShlProtocolReceiver, public ActiveTask {
public:
    ApplicationTask(QueueHandle_t mainTaskQueueHandle, QueueHandle_t haQueueHandle, QueueHandle_t dataRecorderQueueHandle);      
    void setup() override;        
    void loop() override;         
    void handlePacket(const ShlProtocolWithMacAddress &pkt) override;

private:
    static const uint32_t APPLICATION_SYSTEM_TIMER_ID = 1;

    void setupSystemTime(); 

    enum state {
        Idle,
        Measurements,
        HeaterControl,
        HaNotification,
        RtcSync,
        HistoricalDataSync
    };

    //methods to handle each state
    void handleIdleState(ApplicationMessagePacket evt);
    void handleMeasurementsState(ApplicationMessagePacket evt);
    void handleHeaterControlState(ApplicationMessagePacket evt);
    void handleHaNotificationState(ApplicationMessagePacket evt);
    void handleRtcSyncState(ApplicationMessagePacket evt);
    void handleHistoricalDataSyncState(ApplicationMessagePacket evt);
    void sendMeasurementToHa(const MeasurementDataPacket& data);

    // Helper methods
    void logLastResetReason();
    bool is23Pm();
    void sendRtcSyncCommand();
    void updateSystemTime(uint32_t epochTime);
    void updateRtcTime(uint32_t epochTime);
    DateTime getSystemDateTime();
    void collectDataForHaNotification(const MeasurementData& data, bool heaterStatus);
    void calculateHeaterStatus(const MeasurementData& data);
    void haPowerMetersErrorNotification();
    static const char* stateToString(state s);
    static const char* commandTypeToString(ApplicationCommandType type);
    void espNowCommunicationErrorNotification(bool status);
    void espNowCommunicationStatusNotification(bool status, uint16_t totalPower, HeaterStatus heaterState);
    HeaterStatus mapHeaterStatusToHa(uint16_t state);

    uint32_t haQueueFullStreak = 0; 
    uint32_t rtcRetrayCount = 0;
    MeasurementDataPacket lastMeasurementData{};
    
    int last23PmYear = -1;
    int last23PmDayOfYear = -1;
    ShlProtocolTransport transport;
    HeaterFsm heaterFsm;


    HourlySurplusAlgorithm hourlySurplusAlgorithm;
    PowerMeterFsm powerMeterFsm;
    RtcDs3231 rtc;
    ActiveQueueRef<ApplicationMessagePacket> mainTaskQueue;
    ActiveQueueRef<SystemMessagePacket> haQueueRef;
    ActiveQueueRef<SystemMessagePacket> dataRecorderQueueRef;
    SystemTimerT<ApplicationMessagePacket, TimerToApplicationMessage> timer;

    state _state = state::Idle;

    //TODO: temporary, for testing only - remove later
    SDM120CTPowerMeter sdm120ctPowerMeter;
};