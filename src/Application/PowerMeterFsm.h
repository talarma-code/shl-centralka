#pragma once
#include <Arduino.h>
#include "Measurements.h"
#include "ORWE520PowerMeter.h"
#include "SDM120CTPowerMeter.h"

class PowerMeterFsm {
public:


    enum class Next : uint8_t { NextState, Stay, Error };
    struct Result {
        Next next;
        uint16_t delayMs;
    };

    PowerMeterFsm();
    void setup();
    Result messurementReady(MeasurementData& data);

private:
    bool getPowerData(uint32_t& l1TotalEnergyWh, uint32_t& l2TotalEnergyWh, uint32_t& homeTotalEnergyWh);
    void getVoltage( MeasurementData &data);   
    void calculateTotalAndPeriondPowerData(uint32_t l1TotalEnergyWh, uint32_t l2TotalEnergyWh, uint32_t homeTotalEnergyWh, MeasurementData &data); 
    void resetL1PowerOn();
    void resetL1PowerOff();
    void resetL2PowerOn();
    void resetL2PowerOff();

    Result measurmentState(MeasurementData &data);
    Result restartingState();

    void resetCounters();

    enum class MesurmentState : uint8_t {
        Mesurment,
        Restarting
    };

    ORWE520PowerMeter orwe520PowerMeter;
    SDM120CTPowerMeter sdm120ctPowerMeter;

    uint32_t _l1TotalEnergy = 0;
    uint32_t _l2TotalEnergy = 0;
    uint32_t _homeTotalEnergy = 0;

    uint32_t _retryCounter = 0;
    uint32_t _resetCounter = 0;
    uint32_t _l1ReadErrorCount = 0;
    uint32_t _l2ReadErrorCount = 0;

    MesurmentState _state = MesurmentState::Mesurment;
};

