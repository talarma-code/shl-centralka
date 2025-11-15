#include "Application.h"
#include <Arduino.h>

#define LED_PIN 2  // wbudowana dioda LED


Application::Application() : heaterEspNow(LED_PIN) {

}

void Application::setup() {
    Serial.begin(115200);
    transport.begin();
    heaterEspNow.registerTransport(&transport);
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
    }
}

void Application::handlePacket(const MatterLikePacket &pkt, const uint8_t *srcMac) {

}