#pragma once

#include <Arduino.h>
#include <ModbusMaster.h>

class SDM120CTPowerMeter {
public:
    enum class ReadStatus : uint8_t {
        Ok,
        Error
    };

    void setup();
    ReadStatus voltage(float &value, uint8_t slaveId = 1);
    ReadStatus electricCurrent(float &value, uint8_t slaveId = 1);
    ReadStatus activePower(float &value, uint8_t slaveId = 1);
    ReadStatus apparentPower(float &value, uint8_t slaveId = 1);
    ReadStatus reactivePower(float &value, uint8_t slaveId = 1);
    ReadStatus powerFactor(float &value, uint8_t slaveId = 1);
    ReadStatus frequency(float &value, uint8_t slaveId = 1);
    ReadStatus importActiveEnergy(float &value, uint8_t slaveId = 1);
    ReadStatus exportActiveEnergy(float &value, uint8_t slaveId = 1);
    ReadStatus importReactiveEnergy(float &value, uint8_t slaveId = 1);
    ReadStatus exportReactiveEnergy(float &value, uint8_t slaveId = 1);
    ReadStatus totalActiveEnergy(float &value, uint8_t slaveId = 1);
    ReadStatus totalReactiveEnergy(float &value, uint8_t slaveId = 1);

private:
    ModbusMaster node;
    float getFloatValue();
    void modbusError(uint8_t result);
    void debugPrintRequest(uint8_t slave, uint8_t func, uint16_t startAddr, uint16_t count);
};
