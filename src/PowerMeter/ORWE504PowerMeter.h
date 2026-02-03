#pragma once
#include <Arduino.h>
#include <ModbusMaster.h>

class ORWE504PowerMeter {

    public: 
    void setup();
    float voltage(uint8_t slaveId);             //Voltage units 
    float electricCurrent(uint8_t slaveId);     //amperage
    float activePower(uint8_t slaveId);         //Kilowatts
    float reactivePower(uint8_t slaveId);       //Reactive power
    float apparentPower(uint8_t slaveId);       //Apparent power
    float powerFactor(uint8_t slaveId);         //Power factor
    float frequency(uint8_t slaveId);           //hertz
    float totalActivePower(uint8_t slaveId);    //Kilowatt/hours

    private:
    ModbusMaster node;

    void modbusError(uint8_t result);
    float getRegister16Value();
    float getRegister32Value();
};
