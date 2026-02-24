#pragma once
#include <Arduino.h>

struct MeasurementData {
    uint32_t L1Power;
    uint32_t L2Power;
    uint32_t HomePower;

    uint32_t L1TotalPower;
    uint32_t L2TotalPower;
    uint32_t HomeTotalPower;

    uint16_t L1Voltage_x10; 
    uint16_t L2Voltage_x10;
};