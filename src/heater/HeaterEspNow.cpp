#include "HeaterEspNow.h"
#include <Arduino.h> // dla ESP32/ESP-NOW, PWM, pinMode, digitalWrite

HeaterEspNow::HeaterEspNow(int pin) : _pin(pin), _state(false) {
    pinMode(_pin, OUTPUT);
}

void HeaterEspNow::turnOn() {
    _state = true;
    digitalWrite(_pin, HIGH);
}

void HeaterEspNow::turnOff() {
    _state = false;
    digitalWrite(_pin, LOW);
}

bool HeaterEspNow::isOn() const {
    return _state;
}