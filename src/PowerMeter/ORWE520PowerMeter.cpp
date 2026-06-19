#include <Preferences.h>
#include "ORWE520PowerMeter.h"
#include "Log.h"

// Static constant definitions
const float ORWE520PowerMeter::POWER_MULTIPLY = 0.075f;
const float ORWE520PowerMeter::ENERGY_MULTIPLY = 0.00125f;
const uint16_t ORWE520PowerMeter::PULSES_PER_KWH = 800; // 800 pulses per kWh based on meter specifications


const char* NVS_NAMESPACE = "meter";
const char* NVS_KEY_TOTAL_ENERGY = "total_energy";

void ORWE520PowerMeter::setup(float meterEnergyFromSd)
{
    // Configure pulse counter unit
    pcnt_config_t pcnt_config = {
        .pulse_gpio_num = PULSE_PIN,
        .ctrl_gpio_num = -1,                           // No control pin
        .lctrl_mode = PCNT_MODE_KEEP,                  // No effect on low control
        .hctrl_mode = PCNT_MODE_KEEP,                  // No effect on high control
        .pos_mode = PCNT_CHANNEL_EDGE_ACTION_INCREASE, // Increment on rising edge
        .neg_mode = PCNT_CHANNEL_EDGE_ACTION_HOLD,     // No decrement
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
    pcnt_set_filter_value(PCNT_UNIT, 1023); // Maximum filter value to filter out glitches
    pcnt_filter_enable(PCNT_UNIT);

    // Clear counter
    pcnt_counter_clear(PCNT_UNIT);

    // Resume counter
    pcnt_counter_resume(PCNT_UNIT);

    _lastEnergyKWh = 0.0f;
    _totalPulseCount = 0;

    if (meterEnergyFromSd > 0.0f)
    {
        // If current energy is provided, calculate the corresponding pulse count for calibration
        _totalPulseCount = static_cast<uint32_t>(meterEnergyFromSd * PULSES_PER_KWH);
        _lastEnergyKWh = meterEnergyFromSd;
        saveEnergyToNvs(meterEnergyFromSd);
        LOG_INFO("Calibrating power meter from SD, energy: %.2f kWh, pulses %u ", meterEnergyFromSd, _totalPulseCount);
    }
    else 
    {
        // Load last energy from NVS for calibration if available
        float lastEnergy = loadEnergyFromNvs();
        if (lastEnergy > 0.0f)
        {
            _totalPulseCount = static_cast<uint32_t>(lastEnergy * PULSES_PER_KWH);
            _lastEnergyKWh = lastEnergy;
            LOG_INFO("Calibrating power meter from NVS, energy: %.2f kWh, pulses %u ", lastEnergy, _totalPulseCount);
        }
    }
    LOG_INFO("ORWE520 Hardware Pulse Counter initialized on GPIO %d", PULSE_PIN);
}

float ORWE520PowerMeter::totalEnergyKWh()
{

    int16_t currentCount = 0;
    pcnt_counter_pause(PCNT_UNIT);                          // Pause counting to read the current value
    pcnt_get_counter_value(PCNT_UNIT, &currentCount);       // Read the current pulse count from the PCNT unit
    pcnt_counter_clear(PCNT_UNIT);                          // Clear the counter to avoid overflow and prepare for the next reading 
    pcnt_counter_resume(PCNT_UNIT);                         // Resume counting for the next interval

    _totalPulseCount += currentCount;

    float energyKWh = static_cast<float>(_totalPulseCount) * ENERGY_MULTIPLY;

    // Round to 3 decimal places for accuracy
    energyKWh = round(energyKWh * 1000.0f) / 1000.0f;

    if (energyKWh >= _lastEnergyKWh + 1.0f)
    {
        saveEnergyToNvs(energyKWh);
    }
    _lastEnergyKWh = energyKWh;
    return energyKWh;
}

uint32_t ORWE520PowerMeter::totalPulses()
{
    return _totalPulseCount;
}

void ORWE520PowerMeter::saveEnergyToNvs(float meterEnergyFromSd)
{
    Preferences preferences;
    if (preferences.begin(NVS_NAMESPACE, false))  // false - write mode
    {
        preferences.putFloat(NVS_KEY_TOTAL_ENERGY, meterEnergyFromSd);
        preferences.end();
        LOG_INFO("Saved last energy %.2f kWh to NVS", meterEnergyFromSd);
    }
    else
    {
        LOG_ERROR("Failed to open NVS for saving energy");
    }
}


float ORWE520PowerMeter::loadEnergyFromNvs()
{
    Preferences preferences;
    float energy = 0.0f;
    if (preferences.begin(NVS_NAMESPACE, true))
    {
        energy = preferences.getFloat(NVS_KEY_TOTAL_ENERGY, 0.0f);
        preferences.end();
        LOG_INFO("Loaded last energy %.2f kWh from NVS", energy);
    }
    else
    {
        LOG_ERROR("Failed to open NVS for loading energy");
    }
    return energy;
}
