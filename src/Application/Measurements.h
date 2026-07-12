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
    uint16_t L1PowerW;
    uint16_t L2PowerW;
    uint16_t HomePowerW;
};