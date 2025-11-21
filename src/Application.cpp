#include "Application.h"
#include <Arduino.h>

#define LED_PIN 2  // wbudowana dioda LED


Application::Application() : heaterEspNow(LED_PIN) {

}

void Application::setup() {
    Serial.begin(115200);
    transport.begin();
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

    }
}

void Application::handlePacket(const MatterLikePacket &pkt, const uint8_t *srcMac) {
 Serial.println(F("=== MatterLikePacket ==="));

    // Nagłówek
    Serial.println(F("Header:"));
    Serial.print(F("  messageCounter: 0x")); Serial.println(pkt.header.messageCounter, HEX);
    Serial.print(F("  sessionId: 0x")); Serial.println(pkt.header.sessionId, HEX);
    Serial.print(F("  flags: 0x")); Serial.println(pkt.header.flags, HEX);

    // Payload
    Serial.println(F("Payload:"));
    Serial.print(F("  fabricId: 0x")); Serial.println(pkt.payload.fabricId, HEX);
    Serial.print(F("  nodeId: 0x")); Serial.println(pkt.payload.nodeId, HEX);
    Serial.print(F("  endpointId: 0x")); Serial.println(pkt.payload.endpointId, HEX);
    Serial.print(F("  clusterId: 0x")); Serial.println(pkt.payload.clusterId, HEX);
    Serial.print(F("  attributeId: 0x")); Serial.println(pkt.payload.attributeId, HEX);
    Serial.print(F("  commandId: 0x")); Serial.println(pkt.payload.commandId, HEX);
    Serial.print(F("  value: ")); Serial.println(pkt.payload.value);

    Serial.println(F("======================="));
}