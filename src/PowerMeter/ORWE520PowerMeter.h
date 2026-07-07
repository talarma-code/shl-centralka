#pragma once
#include <Arduino.h>
#include <driver/pcnt.h>
#include <Preferences.h>

class ORWE520PowerMeter {

public:
    void setup(float meterEnergyFromSd = 0.0f); // Initialize the power meter, optionally with current power for calibration in case when hardware replacement or reset is needed
    float totalEnergyKWh();              // Total energy in kWh (Zużycie)
    uint32_t totalPulses();              // Total pulse count from the meter


private:
    static const uint8_t PULSE_PIN = 39; // Pin connected to the pulse output of the ORWE520 power meter
    static const pcnt_unit_t PCNT_UNIT = PCNT_UNIT_0;
    static const float POWER_MULTIPLY;      // Converts pulses to kW
    static const float ENERGY_MULTIPLY;     // Converts pulses to kWh
    static const uint16_t PULSES_PER_KWH;
    
    uint32_t _totalPulseCount = 0;
    float _lastEnergyKWh = 0.0f;

    void saveEnergyToNvs(float meterEnergyFromSd); 
    float loadEnergyFromNvs();
};
