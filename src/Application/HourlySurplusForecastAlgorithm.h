// ...existing code...
#pragma once
#ifdef UNIT_TEST
#include <stdint.h>
#include "DateTime_stub.h"
#include "RTClib_stub.h"
#else
#include <Arduino.h>
#include "RTClib.h"
#endif

class HourlySurplusForecastAlgorithm {
public:

    HourlySurplusForecastAlgorithm(uint32_t windowSeconds = 180);

    // l1Wh, l2Wh, homeWh: energy (Wh) produced/consumed during the interval
    // Returns bool: true=heater ON, false=heater OFF (current state after call)
    bool calculatePower(const DateTime& timestamp, uint32_t l1Wh, uint32_t l2Wh, uint32_t homeWh);
private:
    uint32_t lastEpoch;
    uint32_t lastL1;
    uint32_t lastL2;
    uint32_t lastHome;

    // accumulators in Wh for the current hour
    uint32_t accProducedWh;
    uint32_t accConsumedWh;
    int currentHour;

    bool heaterOnState;
    uint32_t windowSeconds;

    static constexpr double heaterPowerWhPerHour = 1500.0; // 1.5 kW = 1500 Wh per hour
    static constexpr double toggleMarginWh = 10.0; // 10 Wh margin

};