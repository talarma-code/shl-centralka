#pragma once
#include "EspNowTransport.h"
#include "IMatterReceiver.h"
#include "ActiveTask.h"
#include "HeaterEspNow.h"
#include "PowerMeter.h"
#include "ORWE504PowerMeter.h"
#include "ActiveQueue.h"
#include "ActivePoolRef.h"
#include "SystemTimer.h"
#include "IntertaskDataModel.h"
#include "RtcDs3231.h"
#include "ApplicationDataModel.h"
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
    ApplicationTask(QueueHandle_t haQueueHandle, QueueHandle_t dataRecorderQueueHandle);      
    void setup() override;        
    void loop() override;         
    void handlePacket(const MatterPacketWithMac &pkt) override;
    MeasurementDataPacket generateRandomMeasurement();
    void setupSystemTime(); 

    void measure();

private:
    void sendMeasurementToHa(const MeasurementDataPacket& data);
    uint32_t haQueueFullStreak = 0; 
    static const uint32_t APPLICATION_SYSTEM_TIMER_ID = 1;
    EspNowTransport transport;
    HeaterEspNow heaterEspNow;
    HeaterFsm heaterFsm;
    PowerMeter powerMeter;
    ORWE504PowerMeter orwe504Meter;
    RtcDs3231 rtc;

    ActiveQueue<ApplicationMessagePacket> mainTaskQueue;
    SystemTimerT<ApplicationMessagePacket, TimerToApplicationMessage> timer;
    ActiveQueueRef<SystemMessagePacket> haQueueRef;
    ActiveQueueRef<SystemMessagePacket> dataRecorderQueueRef;


};