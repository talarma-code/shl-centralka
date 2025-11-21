#pragma once
#include <Arduino.h>
#include "IMatterLikeTransport.h"

class PowerMeter
{

public:
    void setup();
    uint32_t voltage(uint8_t slaveId);          // Voltage units
    uint32_t electricCurrent(uint8_t slaveId);  // amperage
    uint32_t activePower(uint8_t slaveId);      // Kilowatts
    uint32_t frequency(uint8_t slaveId);        // hertz
    uint32_t totalActivePower(uint8_t slaveId); // Kilowatt/hours
    void registerTransport(IMatterLikeTransport *transportLayer);

MatterLikePacket buildVoltageReadPacket(
    uint16_t nodeIdDefault = 0x0001,
    uint8_t endpointIdDefault = 0x01,
    uint32_t fabricIdDefault = 0xA1B2C3D4,
    uint32_t messageCounterDefault = 0x00000001,
    uint16_t sessionIdDefault = 0x0042
);

private:
    IMatterLikeTransport *_transpot;
};