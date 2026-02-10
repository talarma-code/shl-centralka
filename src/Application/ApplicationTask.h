#pragma once
#include "EspNowTransport.h"
#include "IMatterReceiver.h"
#include "ActiveTask.h"
#include "HeaterEspNow.h"
#include "PowerMeter.h"
#include "ORWE504PowerMeter.h"
#include "ActivePoolRef.h"
#include "SystemTimer.h"
#include "IntertaskDataModel.h"
#include "RtcDs3231.h"
#include "HeaterFsm.h"

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
    MeasurementDataPacket generateRandomMeasurement();
    void setupSystemTime(); 

    void measure();

private:
    void logLastResetReason();
    void sendMeasurementToHa(const MeasurementDataPacket& data);
    uint32_t haQueueFullStreak = 0; 
    static const uint32_t APPLICATION_SYSTEM_TIMER_ID = 1;
    EspNowTransport transport;
    HeaterEspNow heaterEspNow;
    HeaterFsm heaterFsm;
    PowerMeter powerMeter;
    ORWE504PowerMeter orwe504Meter;
    RtcDs3231 rtc;

    ActiveQueueRef<ApplicationMessagePacket> mainTaskQueue;
    ActiveQueueRef<SystemMessagePacket> haQueueRef;
    ActiveQueueRef<SystemMessagePacket> dataRecorderQueueRef;
    SystemTimerT<ApplicationMessagePacket, TimerToApplicationMessage> timer;


};