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


class Application : public IMatterReceiver {
public:
    Application(QueueHandle_t haQueueHandle);      
    void setup();         // setup Arduino
    void loop();          // loop Arduino
    void handlePacket(const MatterPacketWithMac &pkt) override;
    MeasurementDataPacket generateRandomMeasurement();
    void setupSystemTime(); 

private:
    static const uint32_t POWER_MEASUREMENT_TIMER_ID = 1;
    EspNowTransport transport;
    HeaterEspNow heaterEspNow;
    PowerMeter powerMeter;
    RtcDs3231 rtc;

    ActiveQueue<TimerEvent> mainTaskQueue;
    SystemTimer powerMeasurementTimer;
    ActiveQueueRef<SystemMessagePacket> haQueueRef;


};