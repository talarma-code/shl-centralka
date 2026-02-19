#pragma once

#include <Arduino.h>
#include <ModbusMaster.h>

class SDM120CTPowerMeter {
public:
    void setup();
    float voltage(uint8_t slaveId = 1);
    float electricCurrent(uint8_t slaveId = 1);
    float activePower(uint8_t slaveId = 1);
    float apparentPower(uint8_t slaveId = 1);
    float reactivePower(uint8_t slaveId = 1);
    float powerFactor(uint8_t slaveId = 1);
    float frequency(uint8_t slaveId = 1);
    float importActiveEnergy(uint8_t slaveId = 1);
    float exportActiveEnergy(uint8_t slaveId = 1);
    float importReactiveEnergy(uint8_t slaveId = 1);
    float exportReactiveEnergy(uint8_t slaveId = 1);
    float totalActiveEnergy(uint8_t slaveId = 1);
    float totalReactiveEnergy(uint8_t slaveId = 1);

private:
    ModbusMaster node;
    float getFloatValue();
    void modbusError(uint8_t result);
    void debugPrintRequest(uint8_t slave, uint8_t func, uint16_t startAddr, uint16_t count);
};
