#include "ORWE520PowerMeter.h"

// Static constant definitions
const float ORWE520PowerMeter::POWER_MULTIPLY = 0.075f;
const float ORWE520PowerMeter::ENERGY_MULTIPLY = 0.00125f;

void ORWE520PowerMeter::setup() {
    // Configure pulse counter unit
    pcnt_config_t pcnt_config = {
        .pulse_gpio_num = PULSE_PIN,
        .ctrl_gpio_num = -1,                          // No control pin
        .lctrl_mode = PCNT_MODE_KEEP,                 // No effect on low control
        .hctrl_mode = PCNT_MODE_KEEP,                 // No effect on high control
        .pos_mode = PCNT_CHANNEL_EDGE_ACTION_INCREASE, // Increment on rising edge
        .neg_mode = PCNT_CHANNEL_EDGE_ACTION_HOLD,    // No decrement
        .counter_h_lim = 32767,                        // Max value for int16_t
        .counter_l_lim = 0,
        .unit = PCNT_UNIT,
        .channel = PCNT_CHANNEL_0,
    };

    // Initialize PCNT unit
    pcnt_unit_config(&pcnt_config);
    
    // Configure glitch filter (internal debounce) - filters noise pulses
    // filter_val is in APB_CLK cycles (80MHz), max 1023 (10-bit value)
    // Maximum filter value = ~12.8 microseconds at 80MHz
    pcnt_set_filter_value(PCNT_UNIT, 1023);  // Maximum filter value to filter out glitches
    pcnt_filter_enable(PCNT_UNIT);
    
    // Clear counter
    pcnt_counter_clear(PCNT_UNIT);
    
    // Resume counter
    pcnt_counter_resume(PCNT_UNIT);
    
    lastCalculationTime = millis();
    
    Serial.println("ORWE520 Hardware Pulse Counter initialized on GPIO " + String(PULSE_PIN) + " (glitch filter enabled)");
}

float ORWE520PowerMeter::currentPowerKW() {
    // Calculate power based on pulse frequency
    // Power (kW) = pulsesPerSecond * POWER_MULTIPLY
    uint32_t currentTime = millis();
    
    // Update pulsesPerSecond calculation every second
    if (currentTime - lastCalculationTime >= 1000) {
        int16_t currentCount = 0;
        pcnt_get_counter_value(PCNT_UNIT, &currentCount);
        
        totalPulseCount = currentCount;
        pulsesPerSecond = currentCount - lastSecondPulseCount;
        lastSecondPulseCount = currentCount;
        lastCalculationTime = currentTime;
    }
    
    float powerKW = static_cast<float>(pulsesPerSecond) * POWER_MULTIPLY;
    
    // Round to 3 decimal places for accuracy
    powerKW = round(powerKW * 1000.0f) / 1000.0f;
    
    return powerKW;
}

float ORWE520PowerMeter::totalEnergyKWh() {
    // Get current counter value from hardware
    int16_t currentCount = 0;
    pcnt_get_counter_value(PCNT_UNIT, &currentCount);
    totalPulseCount = currentCount;
    
    // Total energy is cumulative pulse count * conversion factor
    float energyKWh = static_cast<float>(totalPulseCount) * ENERGY_MULTIPLY;
    
    // Round to 3 decimal places for accuracy
    energyKWh = round(energyKWh * 1000.0f) / 1000.0f;
    
    return energyKWh;
}

void ORWE520PowerMeter::reset() {
    pcnt_counter_clear(PCNT_UNIT);
    totalPulseCount = 0;
    lastSecondPulseCount = 0;
    pulsesPerSecond = 0;
    lastCalculationTime = millis();
}

void ORWE520PowerMeter::update() {
    // This method can be called periodically to update calculations
    // The actual counting is done by hardware, this just updates the power calculation
    currentPowerKW();
}

uint16_t ORWE520PowerMeter::totalPulses() {
    return totalPulseCount;
}
