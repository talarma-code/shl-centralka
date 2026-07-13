#include "PowerMeterFsm.h"  
#include "GlobalTypes.h"
#include "Log.h"

#define L1_POWER_METER_MODBUS_ID 0x01
#define L2_POWER_METER_MODBUS_ID 0x02

PowerMeterFsm::PowerMeterFsm() : orwe520PowerMeter(), sdm120ctPowerMeter() {}

void PowerMeterFsm::setup() {
    orwe520PowerMeter.setup(243.13f); // Initialize ORWE520 power meter with current energy value
    sdm120ctPowerMeter.setup();
}

PowerMeterFsm::Result PowerMeterFsm::messurementReady(MeasurementData &data) {
    switch (_state)
    {
    case MesurmentState::Mesurment:
        return measurmentState(data);
    
    case MesurmentState::Restarting:
        return restartingState();
    
    default:
        return {Next::Error, INTERVAL_100_MS};
    }

}
 
PowerMeterFsm::Result PowerMeterFsm::measurmentState(MeasurementData &data) {
    uint32_t l1TotalEnergyWh, l2TotalEnergyWh, homeTotalEnergyWh;
    if (getEnergyData(l1TotalEnergyWh, l2TotalEnergyWh, homeTotalEnergyWh)) {
        calculateTotalAndPeriondPowerData(l1TotalEnergyWh, l2TotalEnergyWh, homeTotalEnergyWh, data);
        getVoltageAndPower(data);
        resetCounters();
        return {Next::NextState, INTERVAL_100_MS};

    } else {
        _retryCounter++;
        if (_retryCounter > 3) {

            if (_l1ReadErrorCount > 0) {
                resetL1PowerOff();
            } else if (_l2ReadErrorCount > 0) {
                resetL2PowerOff();
            }
            resetCounters();    
            _state = MesurmentState::Restarting;    
            return {Next::Stay, INTERVAL_5_SECONDS_MS};
        }
        return {Next::Stay, INTERVAL_5_SECONDS_MS};
    }
}

PowerMeterFsm::Result PowerMeterFsm::restartingState() {
    // This function can be used to implement any state-specific logic if needed
    resetL1PowerOn();
    resetL2PowerOn();
    _resetCounter++;
    if (_resetCounter > 3) {
        _resetCounter = 0;
        _state = MesurmentState::Mesurment;
        return {Next::Error, INTERVAL_100_MS};
    }
    _state = MesurmentState::Mesurment;   
    return {Next::Stay, INTERVAL_30_SECONDS_MS};
}

bool PowerMeterFsm::getEnergyData(uint32_t& l1TotalEnergyWh, uint32_t& l2TotalEnergyWh, uint32_t& homeTotalEnergyWh) {
    float l1EnergyKwh, l2EnergyKwh;
    auto statusL1 = sdm120ctPowerMeter.importActiveEnergy(l1EnergyKwh, L1_POWER_METER_MODBUS_ID);
    delay(60); 
    auto statusL2 = sdm120ctPowerMeter.importActiveEnergy(l2EnergyKwh, L2_POWER_METER_MODBUS_ID);
    delay(60); 
    
    if (statusL1 != SDM120CTPowerMeter::ReadStatus::Ok) {
        _l1ReadErrorCount++;
    }

    if (statusL2 != SDM120CTPowerMeter::ReadStatus::Ok) {
        _l2ReadErrorCount++;
    }

    if (statusL1 != SDM120CTPowerMeter::ReadStatus::Ok ||
        statusL2 != SDM120CTPowerMeter::ReadStatus::Ok) {
        return false;
    }

    const float homeTotalEnergyKwhRaw = orwe520PowerMeter.totalEnergyKWh();

    constexpr float kWhToWh = 1000.0f;
    l1TotalEnergyWh = static_cast<uint32_t>(l1EnergyKwh * kWhToWh + 0.5f);
    l2TotalEnergyWh = static_cast<uint32_t>(l2EnergyKwh * kWhToWh + 0.5f);
    homeTotalEnergyWh = static_cast<uint32_t>(homeTotalEnergyKwhRaw * kWhToWh + 0.5f);

    LOG_INFO("L1 Energy: %f kWh, L2 Energy: %f kWh, Home Total Energy: %f kWh", l1EnergyKwh, l2EnergyKwh, homeTotalEnergyKwhRaw);

    return true;
}

void PowerMeterFsm::resetL1PowerOn() {
    // TODO: implement L1 power ON action
}

void PowerMeterFsm::resetL1PowerOff() {
    // TODO: implement L1 power OFF action
}

void PowerMeterFsm::resetL2PowerOn() {
    // TODO: implement L2 power ON action
}

void PowerMeterFsm::resetL2PowerOff() {
    // TODO: implement L2 power OFF action
}

void PowerMeterFsm::getVoltageAndPower(MeasurementData &data) {
    float voltageL1, voltageL2, activePowerL1, activePowerL2;
    if (sdm120ctPowerMeter.voltage(voltageL1, L1_POWER_METER_MODBUS_ID) == SDM120CTPowerMeter::ReadStatus::Ok) {
        data.L1Voltage_x10 = static_cast<uint16_t>(voltageL1 * 10);
    } else {
        data.L1Voltage_x10 = 0;
    }

    delay(60);

    if (sdm120ctPowerMeter.activePower(activePowerL1, L1_POWER_METER_MODBUS_ID) == SDM120CTPowerMeter::ReadStatus::Ok) {
        data.L1PowerNowW = activePowerL1;
    } else {
        data.L1PowerNowW = 0;
    }

    delay(60);

    if (sdm120ctPowerMeter.voltage(voltageL2, L2_POWER_METER_MODBUS_ID) == SDM120CTPowerMeter::ReadStatus::Ok) {
        data.L2Voltage_x10 = static_cast<uint16_t>(voltageL2 * 10);
    } else {
        data.L2Voltage_x10 = 0;
    }

    delay(60);

    if (sdm120ctPowerMeter.activePower(activePowerL2, L2_POWER_METER_MODBUS_ID) == SDM120CTPowerMeter::ReadStatus::Ok) {
        data.L2PowerNowW = activePowerL2;
    } else {
        data.L2PowerNowW = 0;
    }
}   

void PowerMeterFsm::calculateTotalAndPeriondPowerData(uint32_t l1TotalEnergyWh, uint32_t l2TotalEnergyWh, uint32_t homeTotalEnergyWh, MeasurementData &data) {
    // capture current time and compute delta from last measurement
    const uint32_t nowMs = millis();
    uint32_t deltaMs = 0;
    if (_lastMeasurementMs != 0) {
        deltaMs = nowMs - _lastMeasurementMs; // unsigned wrap-safe
    }

    if (_l1TotalEnergy == 0) {
        data.L1EnergyInLastTimeWindow = 0;
        data.L1TotalEnergy = l1TotalEnergyWh;
        _l1TotalEnergy = l1TotalEnergyWh;
    } else {
        data.L1EnergyInLastTimeWindow = l1TotalEnergyWh - _l1TotalEnergy;
        _l1TotalEnergy = l1TotalEnergyWh ;
        data.L1TotalEnergy = _l1TotalEnergy;
    }
    if (_l2TotalEnergy == 0) {
        data.L2EnergyInLastTimeWindow = 0;
        data.L2TotalEnergy = l2TotalEnergyWh;
        _l2TotalEnergy = l2TotalEnergyWh;
    } else {
        data.L2EnergyInLastTimeWindow = l2TotalEnergyWh - _l2TotalEnergy;
        _l2TotalEnergy = l2TotalEnergyWh;
        data.L2TotalEnergy = _l2TotalEnergy;
    }

    // home energy
    if (_homeTotalEnergy == 0) {
        data.HomeEnergyInLastTimeWindow = 0;
        data.HomeTotalEnergy = homeTotalEnergyWh;
        _homeTotalEnergy = homeTotalEnergyWh;
    } else {
        data.HomeEnergyInLastTimeWindow = homeTotalEnergyWh - _homeTotalEnergy;
        _homeTotalEnergy = homeTotalEnergyWh;
        data.HomeTotalEnergy = _homeTotalEnergy;
    }

    // compute average power (W) = E_Wh / (delta_hours) = E_Wh * 3600000 / deltaMs
    if (deltaMs == 0) {
        data.L1Power3minW = 0;
        data.L2Power3minW = 0;
        data.HomePower3minW = 0;
    } else {
        data.L1Power3minW = static_cast<uint16_t>((static_cast<uint64_t>(data.L1EnergyInLastTimeWindow) * 3600000) / deltaMs);
        data.L2Power3minW = static_cast<uint16_t>((static_cast<uint64_t>(data.L2EnergyInLastTimeWindow) * 3600000) / deltaMs);
        data.HomePower3minW = static_cast<uint16_t>((static_cast<uint64_t>(data.HomeEnergyInLastTimeWindow) * 3600000) / deltaMs);
    }

    _lastMeasurementMs = nowMs;

    LOG_INFO("Powers - L1: %u Wh, L2: %u Wh, Home: %u Wh | Totals - L1: %u Wh, L2: %u Wh, Home: %u Wh | InstPower - L1: %u W, L2: %u W, Home: %u W", 
             data.L1EnergyInLastTimeWindow, data.L2EnergyInLastTimeWindow, data.HomeEnergyInLastTimeWindow, 
             data.L1TotalEnergy, data.L2TotalEnergy, data.HomeTotalEnergy,
             data.L1Power3minW, data.L2Power3minW, data.HomePower3minW);
}


void PowerMeterFsm::resetCounters() {
    _retryCounter = 0;
    _l1ReadErrorCount = 0;
    _l2ReadErrorCount = 0;
}

