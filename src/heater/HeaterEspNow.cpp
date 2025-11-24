#include "HeaterEspNow.h"
#include <Arduino.h> // dla ESP32/ESP-NOW, PWM, pinMode, digitalWrite
#include "MatterLike.h"


#define HEATER_DEVICE_BASMENT 1
#define INTERNAL_HEATER_1 1
#define INTERNAL_HEATER_2_LED 2

static const uint8_t MAC_HEATER[]  = {0x74, 0x61, 0x6C, 0x61, 0x72, 0x31}; // talar1 - heater


HeaterEspNow::HeaterEspNow(int pin) : _pin(pin), _state(false)  {
    pinMode(_pin, OUTPUT);
}

void HeaterEspNow::registerTransport(IMatterLikeTransport* transportLayer) {
    _transpot = transportLayer;
}

void HeaterEspNow::turnOn() {
    _state = true;
    digitalWrite(_pin, HIGH);
    MatterLikePacket packet = MatterLike::createTurnOnPacket(HEATER_DEVICE_BASMENT, INTERNAL_HEATER_1);
    if (_transpot){
        _transpot->send(MAC_HEATER, packet);
    }
    else {
        Serial.println("HeaterEspNow::_transpot not registered ");
    }
    
}

void HeaterEspNow::turnOff() {
    _state = false;
    digitalWrite(_pin, LOW);
    MatterLikePacket packet = MatterLike::createTurnOffPacket(HEATER_DEVICE_BASMENT, INTERNAL_HEATER_1);
        if (_transpot){
        _transpot->send(MAC_HEATER, packet);
    }
    else {
        Serial.println("HeaterEspNow::_transpot not registered ");
    }
}

bool HeaterEspNow::isOn() const {
    return _state;
}