// ShortTermSurplusForecastAlgorithm
// Controls heater based on PV surplus forecast for the next measurement window (e.g. 3 minutes).
// All calculations in integer Wh.

#pragma once
#include <Arduino.h>
#include "RTClib.h"

class ShortTermSurplusForecastAlgorithm {
public:
    ShortTermSurplusForecastAlgorithm(uint32_t windowSeconds = 180);

    void calculatePower(const DateTime& timestamp, uint32_t l1Wh, uint32_t l2Wh, uint32_t homeWh);
    uint32_t l1Power();
    uint32_t l2Power();
    uint32_t homePowerConsumption();

    void enableHeater();
    void disableHeater();

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
