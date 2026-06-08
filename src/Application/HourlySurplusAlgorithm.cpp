/*
    HourlySurplusAlgorithm

    This algorithm manages a heater based on photovoltaic (PV) energy production
    and home consumption within the current clock hour.

    - The method calculatePower() is called every few minutes with energy
      produced/consumed in Wh for the last measurement interval (windowSeconds).
    - For each clock hour it accumulates total PV energy produced (L1 + L2)
      and total home energy consumed, both in Wh.
    - At each call it checks whether the accumulated surplus in the current
      hour (producedWh - consumedWh) is sufficient to cover the heater's
      energy usage for the next interval
      (heaterPowerWhPerHour * windowSeconds / 3600).
    - If the surplus is sufficient, the algorithm allows the heater to be ON;
      otherwise it forces it OFF.
    - All calculations are done in integer Wh (no floating point).
*/
#ifdef UNIT_TEST
#include <stdint.h>
#else
#include <Arduino.h>
#endif

#include "HourlySurplusAlgorithm.h"
#include "Log.h"

HourlySurplusAlgorithm::HourlySurplusAlgorithm(uint32_t windowSeconds)
    : lastEpoch(0), lastL1(0), lastL2(0), lastHome(0), accProducedWh(0), accConsumedWh(0), currentHour(-1), heaterOnState(false), windowSeconds(windowSeconds) {}


bool HourlySurplusAlgorithm::calculatePower(const DateTime& timestamp, uint32_t l1Wh, uint32_t l2Wh, uint32_t homeWh) {
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
    LOG_INFO("Hour: %02d, Accumulated Produced: %u Wh, Accumulated Consumed: %u Wh, Heater needs: %u Wh", hour, accProducedWh, accConsumedWh, heaterWhThisInterval);
    if (accProducedWh >= accConsumedWh + heaterWhThisInterval) {
        heaterOnState = true;
    } else {
        heaterOnState = false;
    }
    return heaterOnState;
}
