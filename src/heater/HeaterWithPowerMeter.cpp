#include "HeaterWithPowerMeter.h"

#include <Arduino.h>

#include "MatterLike.h"

namespace {
constexpr uint16_t HEATER_NODE_ID = 0x0001;
constexpr uint8_t INTERNAL_ENDPOINT_PRIMARY = 0x01;
constexpr uint8_t MAC_HEATER[6] = {0x74, 0x61, 0x6C, 0x61, 0x72, 0x31};
}

HeaterWithPowerMeter::HeaterWithPowerMeter(int pin) : _pin(pin) {
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
}

void HeaterWithPowerMeter::registerTransport(IMatterLikeTransport* transportLayer) {
    _transport = transportLayer;
}

void HeaterWithPowerMeter::turnOn() {
    _state = true;
    digitalWrite(_pin, HIGH);
    MatterLikePacket packet = MatterLike::createTurnOnPacket(HEATER_NODE_ID, INTERNAL_ENDPOINT_PRIMARY);
    sendPacket(packet);
}

void HeaterWithPowerMeter::turnOff() {
    _state = false;
    digitalWrite(_pin, LOW);
    MatterLikePacket packet = MatterLike::createTurnOffPacket(HEATER_NODE_ID, INTERNAL_ENDPOINT_PRIMARY);
    sendPacket(packet);
}

bool HeaterWithPowerMeter::isOn() const {
    return _state;
}

uint32_t HeaterWithPowerMeter::voltage(uint8_t endpointId) {
    MatterLikePacket packet = MatterLike::createReadElectricalPacket(HEATER_NODE_ID, endpointId, ATTR_EM_RMS_VOLTAGE);
    sendPacket(packet);
    return 2300;
}

uint32_t HeaterWithPowerMeter::totalActivePower(uint8_t endpointId) {
    MatterLikePacket packet = MatterLike::createReadTotalActivePowerPacket(HEATER_NODE_ID, endpointId);
    sendPacket(packet);
    return 123456;
}

void HeaterWithPowerMeter::sendPacket(const MatterLikePacket& packet) const {
    if (!_transport) {
        Serial.println("HeaterWithPowerMeter::transport not registered");
        return;
    }
    _transport->send(MAC_HEATER, packet);
}
