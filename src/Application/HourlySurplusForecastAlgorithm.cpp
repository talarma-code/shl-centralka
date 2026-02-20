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
    // Reset accumulators at the start of each clock hour
    int hour = timestamp.hour();
    uint32_t producedWhInterval = l1Wh + l2Wh;
    uint32_t consumedWhInterval = homeWh;

    if (hour != currentHour) {
        accProducedWh = producedWhInterval;
        accConsumedWh = consumedWhInterval;
        currentHour = hour;
    } else {
        accProducedWh += producedWhInterval;
        accConsumedWh += consumedWhInterval;
    }

    // How much energy would be used by the heater in this interval?
    uint32_t heaterWhThisInterval = heaterPowerWhPerHour * windowSeconds / 3600;

    // If accumulated surplus is enough for this interval, allow heater ON
    if (accProducedWh >= accConsumedWh + heaterWhThisInterval) {
        heaterOnState = true;
    } else {
        heaterOnState = false;
    }
    return heaterOnState;
}

