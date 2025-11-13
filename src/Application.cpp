#include "Application.h"
#include <Arduino.h>

#define LED_PIN 2  // wbudowana dioda LED

Application::Application()
    : heater(LED_PIN)       // przypisanie pinu grzałki
{}

void Application::setup() {
    Serial.begin(115200);
    heater.turnOff();
}

void Application::loop() {
    Serial.println("szybko mrugacz");
    heater.turnOn();
    delay(3000);
    heater.turnOff();
    delay(3000);
}