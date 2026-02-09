#pragma once
#include <Arduino.h>
#include <ModbusMaster.h>

class DDS661PowerMeter {

    public: 
    void setup();
    float voltage(uint8_t slaveId);             //Voltage units 
    float electricCurrent(uint8_t slaveId);     //amperage
    float activePower(uint8_t slaveId);         //Kilowatts
    float frequency(uint8_t slaveId);           //hertz
    float totalActivePower(uint8_t slaveId);    //Kilowatt/hours

    private:
    ModbusMaster node;

    void modbusError(uint8_t result);
    float getFloatValue();
};
