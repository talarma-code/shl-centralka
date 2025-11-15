#include "HeaterEspNow.h"
#include <Arduino.h> // dla ESP32/ESP-NOW, PWM, pinMode, digitalWrite
#include "../MatterLikeProtocol/MatterLike.h"


#define HEATER_DEVICE_BASMENT 1
#define INTERNAL_HEATER_1 1
#define INTERNAL_HEATER_2_LED 2

//uint8_t MAC_SHL_CENTRALLA[] = { 0x40, 0x91, 0x51, 0x20, 0xC1, 0x98 };
uint8_t MAC_SHL_CENTRALLA[] = { 0x40, 0x91, 0x51, 0x20, 0xC1, 0x98 };

//uint8_t MAC_SHL_STEROWNIK_GRZALKI[] = { 0x88, 0x57, 0x21, 0xC0, 0x8F, 0x28 };
uint8_t MAC_SHL_STEROWNIK_GRZALKI[] = { 0x44, 0x1D, 0x64, 0xFA, 0x2B, 0x28};



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
        _transpot->send(MAC_SHL_STEROWNIK_GRZALKI, packet);
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
        _transpot->send(MAC_SHL_STEROWNIK_GRZALKI, packet);
    }
    else {
        Serial.println("HeaterEspNow::_transpot not registered ");
    }
}

bool HeaterEspNow::isOn() const {
    return _state;
}