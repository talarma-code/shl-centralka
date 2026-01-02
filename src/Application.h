#pragma once
#include "EspNowTransport.h"
#include "IMatterReceiver.h"
#include "HeaterEspNow.h"
#include "PowerMeter.h"
#include "ActiveQueue.h"
#include "SystemTimer.h"


class Application : public IMatterReceiver {
public:
    Application();        // konstruktor
    void setup();         // setup Arduino
    void loop();          // loop Arduino
    void handlePacket(const MatterPacketWithMac &pkt) override;

private:
    static const uint32_t POWER_MEASUREMENT_TIMER_ID = 1;
    EspNowTransport transport;
    HeaterEspNow heaterEspNow;
    PowerMeter powerMeter;

    ActiveQueue<TimerEvent> mainTaskQueue;
    SystemTimer powerMeasurementTimer;


};