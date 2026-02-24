#pragma once
#include <Arduino.h>
#include "Measurements.h"
#include "ORWE520PowerMeter.h"
#include "SDM120CTPowerMeter.h"

class PowerMeterFsm {
public:
    enum class Status : uint8_t {
        MesurmentOk,
        RetryMesurment,
        Error
    };
    PowerMeterFsm();
    void setup();
    Status messurementReady(MeasurementData& data);

private:
    bool getPowerData(float& l1Energy, float& l2Energy, float& homeTotalEnergy);
    void getVoltage( MeasurementData &data);   
    void calculateTotalAndPeriondPowerData(float l1Energy, float l2Energy, float homeTotalEnergy, MeasurementData &data); 

    ORWE520PowerMeter orwe520PowerMeter;
    SDM120CTPowerMeter sdm120ctPowerMeter;

    uint32_t _l1TotalPower = 0;
    uint32_t _l2TotalPower = 0;
    uint32_t _homeTotalPower = 0;

    uint32_t _retryCounter = 0;
};

