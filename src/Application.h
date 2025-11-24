#pragma once
#include "EspNowTransport.h"
#include "IMatterReceiver.h"
#include "HeaterEspNow.h"
#include "PowerMeter.h"


class Application : public IMatterReceiver {
public:
    Application();        // konstruktor
    void setup();         // setup Arduino
    void loop();          // loop Arduino
    void handlePacket(const MatterLikePacket &pkt, const uint8_t *srcMac) override;

private:
    EspNowTransport transport;
    HeaterEspNow heaterEspNow;
    PowerMeter powerMeter;

};