#pragma once
#include <Arduino.h>
#include <driver/pcnt.h>

class ORWE520PowerMeter {

public:
    void setup();
    float currentPowerKW();              // Current power in kW (Aktualna moc)
    float totalEnergyKWh();              // Total energy in kWh (Zużycie)
    uint16_t totalPulses();              // Total pulse count from the meter
    void reset();                        // Reset total energy counter
    void update();                       // Call periodically to update power calculation

private:
    static const uint8_t PULSE_PIN = 22;
    static const pcnt_unit_t PCNT_UNIT = PCNT_UNIT_0;
    static const float POWER_MULTIPLY;      // Converts pulses to kW
    static const float ENERGY_MULTIPLY;     // Converts pulses to kWh
    
    volatile int16_t pulsesPerSecond = 0;
    uint32_t lastCalculationTime = 0;
    int16_t lastSecondPulseCount = 0;
    int16_t totalPulseCount = 0;
};
