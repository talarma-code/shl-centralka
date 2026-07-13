#pragma once
#include <Arduino.h>

struct MeasurementData {
    uint32_t L1EnergyInLastTimeWindow;
    uint32_t L2EnergyInLastTimeWindow;
    uint32_t HomeEnergyInLastTimeWindow;

    uint32_t L1TotalEnergy;
    uint32_t L2TotalEnergy;
    uint32_t HomeTotalEnergy;

    uint16_t L1Voltage_x10; 
    uint16_t L2Voltage_x10;
    // Average power over last measurement window (watts)
    uint16_t L1Power3minW;
    uint16_t L2Power3minW;
    uint16_t HomePower3minW;

    uint16_t L1PowerNowW;
    uint16_t L2PowerNowW;
};