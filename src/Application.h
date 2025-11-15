#pragma once
#include "NetworkTransport/EspNowTransport.h"
#include "NetworkTransport/IMatterReceiver.h"
#include "Heater/HeaterEspNow.h"


class Application : public IMatterReceiver {
public:
    Application();        // konstruktor
    void setup();         // setup Arduino
    void loop();          // loop Arduino
    void handlePacket(const MatterLikePacket &pkt, const uint8_t *srcMac) override;

private:
    EspNowTransport transport;
    HeaterEspNow heaterEspNow;

};