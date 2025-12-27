#include "Application.h"
#include <Arduino.h>
#include "MatterLikeDebugger.h"
#include "SystemDebuger.h"

#define LED_PIN 2  // wbudowana dioda LED
static const uint8_t MAC_LOCAL_HEATER[]  = {0x74, 0x61, 0x6C, 0x61, 0x72, 0x31}; // talar1 - heater
static const uint8_t MAC_CENTRALKA[]   = {0x74, 0x61, 0x6C, 0x61, 0x72, 0x30}; // talar0 - centrala

Application::Application() : heaterEspNow(LED_PIN) {

}

void Application::setup() {
    Serial.begin(115200);
    Serial.println("\n=== SHL Centralka Start ===");
    
    transport.begin(MAC_CENTRALKA, MAC_LOCAL_HEATER);
    transport.onPacketReceived(this);
    heaterEspNow.registerTransport(&transport);
    powerMeter.registerTransport(&transport);
}

void Application::loop() {
    
    if (Serial.available() > 0) {
        char characterRecived = Serial.read();   // Odczytaj 1 bajt

        if (characterRecived == 'o') {
            Serial.println("odebralem i wyslam ramke On");
            heaterEspNow.turnOn();
        }

        if (characterRecived == 'f') {
            Serial.println("odebralem i wyslam ramke Off");
            heaterEspNow.turnOff();
            //action Off
        }
        if (characterRecived == 'v') {
            Serial.println("odebralem i wyslam ramke v");
            powerMeter.voltage(1);
            //action Off
        }

        SystemDebuger::printSystemStats();

    }
}

void Application::handlePacket(const MatterPacketWithMac &pkt) {
 Serial.println(F("=== MatterLikePacket ==="));
    MatterLikeDebugger::print(pkt);
}