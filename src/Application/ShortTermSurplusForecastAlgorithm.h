// ShortTermSurplusForecastAlgorithm
// Controls heater based on PV surplus forecast for the next measurement window (e.g. 3 minutes).
// All calculations in integer Wh.

#pragma once
#ifdef UNIT_TEST
#include <stdint.h>
#include "DateTime_stub.h"
#include "RTClib_stub.h"
#else
#include <Arduino.h>
#include "RTClib.h"
#endif

class ShortTermSurplusForecastAlgorithm {
public:
    ShortTermSurplusForecastAlgorithm(uint32_t windowSeconds = 180);

    // Returns bool: true=heater ON, false=heater OFF (current state after call)
    bool calculatePower(const DateTime& timestamp, uint32_t l1Wh, uint32_t l2Wh, uint32_t homeWh);

private:
    uint32_t lastEpoch;
    uint32_t lastL1;
    uint32_t lastL2;
    uint32_t lastHome;
    uint32_t accProducedWh;
    uint32_t accConsumedWh;
    int currentHour;
    bool heaterOnState;
    uint32_t windowSeconds;

    static constexpr uint32_t heaterPowerWhPerHour = 1500; // 1.5 kW = 1500 Wh per hour
    static constexpr uint32_t toggleMarginWh = 100; // 100 Wh margin
};
