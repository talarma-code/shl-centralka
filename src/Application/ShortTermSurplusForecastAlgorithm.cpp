/*
    ShortTermSurplusForecastAlgorithm

    This algorithm manages a heater based on photovoltaic (PV) energy production and home consumption.
    It forecasts if there will be enough surplus energy for the heater to run during the next measurement window (e.g. 3 minutes).
    - The method calculatePower() is called every few minutes with energy produced/consumed in Wh for the interval.
    - The algorithm accumulates energy produced and consumed in the current hour.
    - It projects the energy balance for the next window, assuming current rates persist.
    - The heater (1.5 kW) is enabled if there is or will be sufficient surplus energy for the next window.
    - Hysteresis is used to avoid frequent toggling.
    - All calculations are done in integer Wh (no floating point).
    - The heater's consumption is only added to the projection if it is currently off.
*/

#include "ShortTermSurplusForecastAlgorithm.h"
#include <Arduino.h>

ShortTermSurplusForecastAlgorithm::ShortTermSurplusForecastAlgorithm(uint32_t windowSeconds)
    : lastEpoch(0), lastL1(0), lastL2(0), lastHome(0), accProducedWh(0), accConsumedWh(0), currentHour(-1), heaterOnState(false), windowSeconds(windowSeconds) {}

uint32_t ShortTermSurplusForecastAlgorithm::l1Power() {
    return 0;
}

uint32_t ShortTermSurplusForecastAlgorithm::l2Power() {
    return 0;
}

uint32_t ShortTermSurplusForecastAlgorithm::homePowerConsumption() {
    return 0;
}

void ShortTermSurplusForecastAlgorithm::enableHeater() {
    if (!heaterOnState) {
        heaterOnState = true;
        Serial.println("Heater enabled");
    }
}

void ShortTermSurplusForecastAlgorithm::disableHeater() {
    if (heaterOnState) {
        heaterOnState = false;
        Serial.println("Heater disabled");
    }
}

void ShortTermSurplusForecastAlgorithm::calculatePower(const DateTime& timestamp, uint32_t l1Wh, uint32_t l2Wh, uint32_t homeWh) {
    uint32_t now = (uint32_t)timestamp.unixtime();

    // Total energy in the interval (Wh)
    uint32_t producedWhInterval = l1Wh + l2Wh;
    uint32_t consumedWhInterval = homeWh;

    if (lastEpoch == 0) {
        lastEpoch = now;
        currentHour = timestamp.hour();
        accProducedWh = producedWhInterval;
        accConsumedWh = consumedWhInterval;
        return;
    }

    if (now <= lastEpoch) return;


    // Accumulate for hour (optional, for stats)
    accProducedWh += producedWhInterval;
    accConsumedWh += consumedWhInterval;

    // Forecast for next window (use windowSeconds for rate calculation)
    uint32_t producedRatePerSec = (windowSeconds > 0) ? (producedWhInterval / windowSeconds) : 0; // Wh/s
    uint32_t consumedRatePerSec = (windowSeconds > 0) ? (consumedWhInterval / windowSeconds) : 0; // Wh/s

    uint32_t projectedProduced = producedRatePerSec * windowSeconds;
    uint32_t projectedConsumed = consumedRatePerSec * windowSeconds;
    uint32_t projectedHeaterWh = heaterPowerWhPerHour * windowSeconds / 3600;

    int32_t projectedNetIfHeaterOn;
    if (!heaterOnState) {
        // If the heater is off, add its projected consumption to the forecast
        projectedNetIfHeaterOn = (int32_t)(projectedProduced) - (int32_t)(projectedConsumed + projectedHeaterWh);
    } else {
        // If the heater is already on, its consumption is already included in homeWh
        projectedNetIfHeaterOn = (int32_t)(projectedProduced) - (int32_t)(projectedConsumed);
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
        enableHeater();
    } else {
        if (!heaterOnState && projectedNetIfHeaterOn >= (int32_t)toggleMarginWh) {
            enableHeater();
        } else if (heaterOnState && projectedNetIfHeaterOn < -(int32_t)toggleMarginWh) {
            disableHeater();
        }
    }

    lastEpoch = now;
}
