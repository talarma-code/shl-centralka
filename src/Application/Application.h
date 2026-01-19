#pragma once
#include "EspNowTransport.h"
#include "IMatterReceiver.h"
#include "HeaterEspNow.h"
#include "PowerMeter.h"
#include "ActiveQueue.h"
#include "ActivePoolRef.h"
#include "SystemTimer.h"
#include "IntertaskDataModel.h"
#include "RtcDs3231.h"
#include "ApplicationDataModel.h"

class TimerToApplicationMessage {
    public:
    ApplicationMessagePacket operator()(const TimerEvent& t) const noexcept {
        ApplicationMessagePacket m{};
        m.type = ApplicationCommandType::Timer;
        m.payload.timerEvent.timerId = t.timerId;
        return m;
    }
};


class Application : public IMatterReceiver {
public:
    Application(QueueHandle_t haQueueHandle, QueueHandle_t dataRecorderQueueHandle);      
    void setup();         // setup Arduino
    void loop();          // loop Arduino
    void handlePacket(const MatterPacketWithMac &pkt) override;
    MeasurementDataPacket generateRandomMeasurement();
    void setupSystemTime(); 

private:
    static const uint32_t APPLICATION_SYSTEM_TIMER_ID = 1;
    EspNowTransport transport;
    HeaterEspNow heaterEspNow;
    PowerMeter powerMeter;
    RtcDs3231 rtc;

    ActiveQueue<ApplicationMessagePacket> mainTaskQueue;
    SystemTimerT<ApplicationMessagePacket, TimerToApplicationMessage> timer;
    ActiveQueueRef<SystemMessagePacket> haQueueRef;
    ActiveQueueRef<SystemMessagePacket> dataRecorderQueueRef;


};