/*
    HourlySurplusForecastAlgorithm

    This algorithm manages a heater based on photovoltaic (PV) energy production and home consumption.
    It is designed for Polish prosumer hourly netting (Tauron):
    - The method calculatePower() is called every few minutes with energy produced/consumed in Wh for the interval.
    - The algorithm accumulates energy produced and consumed in the current hour.
    - It projects the energy balance to the end of the hour, assuming current rates persist.
    - The heater (1.5 kW) is enabled if there is or will be sufficient surplus energy in the current hour.
    - Hysteresis is used to avoid frequent toggling.
    - All calculations are done in integer Wh (no floating point).
    - The heater's consumption is only added to the projection if it is currently off.
*/
#ifdef UNIT_TEST
#include <stdint.h>
#else
#include <Arduino.h>
#endif

#include "HourlySurplusForecastAlgorithm.h"

HourlySurplusForecastAlgorithm::HourlySurplusForecastAlgorithm(uint32_t windowSeconds)
    : lastEpoch(0), lastL1(0), lastL2(0), lastHome(0), accProducedWh(0), accConsumedWh(0), currentHour(-1), heaterOnState(false), windowSeconds(windowSeconds) {}


bool HourlySurplusForecastAlgorithm::calculatePower(const DateTime& timestamp, uint32_t l1Wh, uint32_t l2Wh, uint32_t homeWh) {
    uint32_t now = (uint32_t)timestamp.unixtime();

    // Total energy in the interval (Wh)
    uint32_t producedWhInterval = l1Wh + l2Wh;
    uint32_t consumedWhInterval = homeWh;

    if (lastEpoch == 0) {
        lastEpoch = now;
        currentHour = timestamp.hour();
        accProducedWh = producedWhInterval;
        accConsumedWh = consumedWhInterval;
        return heaterOnState;
    }

    if (now <= lastEpoch) return heaterOnState;


    // Split the interval energy across hour boundaries proportionally (integer, rounded to 1 Wh)
    uint32_t dtSec = windowSeconds; // Use windowSeconds for rate calculation
    if (dtSec == 0) return heaterOnState;
    uint32_t segStart = lastEpoch;
    uint32_t segEnd = now;
    uint32_t producedLeft = producedWhInterval;
    uint32_t consumedLeft = consumedWhInterval;
    while (segStart < segEnd) {
        uint32_t hourIndex = segStart / 3600;
        uint32_t hourEnd = (hourIndex + 1) * 3600;
        uint32_t thisSegEnd = (hourEnd < segEnd) ? hourEnd : segEnd;
        uint32_t segSeconds = thisSegEnd - segStart;

        uint32_t segProduced = (uint32_t)(((uint64_t)producedWhInterval * segSeconds) / dtSec);
        uint32_t segConsumed = (uint32_t)(((uint64_t)consumedWhInterval * segSeconds) / dtSec);

        // Correction for the last segment (to ensure the sum matches the interval)
        if (thisSegEnd == segEnd) {
            segProduced = producedLeft;
            segConsumed = consumedLeft;
        }

        int segHour = (int)((segStart / 3600) % 24);
        if (segHour != currentHour) {
            // New hour: reset accumulators for the new hour
            accProducedWh = 0;
            accConsumedWh = 0;
            currentHour = segHour;
        }

        accProducedWh += segProduced;
        accConsumedWh += segConsumed;

        producedLeft -= segProduced;
        consumedLeft -= segConsumed;
        segStart = thisSegEnd;
    }

    // Projection to the end of the hour (integer, no fractions)
    uint32_t producedSoFar = accProducedWh;
    uint32_t consumedSoFar = accConsumedWh;

    uint32_t producedRatePerSec = (windowSeconds > 0) ? (producedWhInterval / windowSeconds) : 0; // Wh/s
    uint32_t consumedRatePerSec = (windowSeconds > 0) ? (consumedWhInterval / windowSeconds) : 0; // Wh/s

    uint32_t secondsUntilHourEnd = (uint32_t)(((now / 3600) + 1) * 3600 - now);
    uint32_t projectedProducedRemaining = producedRatePerSec * secondsUntilHourEnd;
    uint32_t projectedConsumedRemaining = consumedRatePerSec * secondsUntilHourEnd;
    uint32_t projectedHeaterWh = heaterPowerWhPerHour * secondsUntilHourEnd / 3600;

    int32_t projectedNetIfHeaterOn;
    if (!heaterOnState) {
        // If the heater is off, add its projected consumption to the forecast
        projectedNetIfHeaterOn = (int32_t)(producedSoFar + projectedProducedRemaining) - (int32_t)(consumedSoFar + projectedConsumedRemaining + projectedHeaterWh);
    } else {
        // If the heater is already on, its consumption is already included in homeWh
        projectedNetIfHeaterOn = (int32_t)(producedSoFar + projectedProducedRemaining) - (int32_t)(consumedSoFar + projectedConsumedRemaining);
    }

    // Short-term condition: does production cover consumption (+ heater if off)
    uint32_t curProdWhPerHour = producedRatePerSec * 3600;
    uint32_t curConsWhPerHour = consumedRatePerSec * 3600;

    bool canEnable;
    if (!heaterOnState) {
        // If heater is off, check if production covers consumption + heater
        canEnable = (curProdWhPerHour >= (curConsWhPerHour + heaterPowerWhPerHour));
    } else {
        // If heater is on, check if production covers current consumption (including heater)
        canEnable = (curProdWhPerHour >= curConsWhPerHour);
    }

    if (canEnable) {
        if (!heaterOnState) {
            heaterOnState = true;
        }
    } else {
        if (!heaterOnState && projectedNetIfHeaterOn >= (int32_t)toggleMarginWh) {
            heaterOnState = true;
        } else if (heaterOnState && projectedNetIfHeaterOn < -(int32_t)toggleMarginWh) {
            heaterOnState = false;
        }
    }
    lastEpoch = now;
    return heaterOnState;
}

