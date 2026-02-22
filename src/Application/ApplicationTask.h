#pragma once
#include "EspNowTransport.h"
#include "IMatterReceiver.h"
#include "ActiveTask.h"
#include "HeaterEspNow.h"
#include "PowerMeter.h"
//#include "ORWE504PowerMeter.h"
#include "ActivePoolRef.h"
#include "SystemTimer.h"
#include "IntertaskDataModel.h"
#include "RtcDs3231.h"
#include "HeaterFsm.h"
#include "ORWE520PowerMeter.h"
#include "SDM120CTPowerMeter.h"

class TimerToApplicationMessage {
    public:
    ApplicationMessagePacket operator()(const TimerEvent& t) const noexcept {
        ApplicationMessagePacket m{};
        m.type = ApplicationCommandType::Timer;
        m.payload.timerEvent.timerId = t.timerId;
        return m;
    }
};


class ApplicationTask : public IMatterReceiver, public ActiveTask {
public:
    ApplicationTask(QueueHandle_t mainTaskQueueHandle, QueueHandle_t haQueueHandle, QueueHandle_t dataRecorderQueueHandle);      
    void setup() override;        
    void loop() override;         
    void handlePacket(const MatterPacketWithMac &pkt) override;

private:
    static const uint32_t APPLICATION_SYSTEM_TIMER_ID = 1;

    MeasurementDataPacket generateRandomMeasurement();
    void setupSystemTime(); 

    enum state {
        Idle,
        Measurements,
        HeaterControl,
        HaNotification,
        RtcSync,
        HistoricalDataSync
    };

    void logLastResetReason();
    bool is23Pm();
    void handleIdleState(ApplicationMessagePacket evt);
    void handleMeasurementsState(ApplicationMessagePacket evt);
    void handleHeaterControlState(ApplicationMessagePacket evt);
    void handleHaNotificationState(ApplicationMessagePacket evt);
    void handleRtcSyncState(ApplicationMessagePacket evt);
    void handleHistoricalDataSyncState(ApplicationMessagePacket evt);
    void sendMeasurementToHa(const MeasurementDataPacket& data);

    void sendRtcSyncCommand();

    uint32_t haQueueFullStreak = 0; 
    uint32_t rtcRetrayCount = 0;

    
    int last23PmYear = -1;
    int last23PmDayOfYear = -1;
    EspNowTransport transport;
    HeaterEspNow heaterEspNow;
    HeaterFsm heaterFsm;
    PowerMeter powerMeter;
    RtcDs3231 rtc;

    ActiveQueueRef<ApplicationMessagePacket> mainTaskQueue;
    ActiveQueueRef<SystemMessagePacket> haQueueRef;
    ActiveQueueRef<SystemMessagePacket> dataRecorderQueueRef;
    SystemTimerT<ApplicationMessagePacket, TimerToApplicationMessage> timer;

    ORWE520PowerMeter orwe520PowerMeter;
    SDM120CTPowerMeter sdm120ctPowerMeter;

    state _state = state::Idle;
    static const uint32_t INTERVAL_3_MINUTES_MS = 180000; 
    static const uint32_t INTERVAL_5_SECONDS_MS = 5000;


};