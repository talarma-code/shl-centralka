#include "PowerMeterFsm.h"  
#include "GlobalTypes.h"
#include "Log.h"

#define L1_POWER_METER_MODBUS_ID 0x01
#define L2_POWER_METER_MODBUS_ID 0x02

PowerMeterFsm::PowerMeterFsm() : orwe520PowerMeter(), sdm120ctPowerMeter() {}

void PowerMeterFsm::setup() {
    orwe520PowerMeter.setup();
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
    if (getPowerData(l1TotalEnergyWh, l2TotalEnergyWh, homeTotalEnergyWh)) {
        calculateTotalAndPeriondPowerData(l1TotalEnergyWh, l2TotalEnergyWh, homeTotalEnergyWh, data);
        getVoltage(data);
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

bool PowerMeterFsm::getPowerData(uint32_t& l1TotalEnergyWh, uint32_t& l2TotalEnergyWh, uint32_t& homeTotalEnergyWh) {
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

void PowerMeterFsm::getVoltage(MeasurementData &data) {
    float voltageL1, voltageL2;
    if (sdm120ctPowerMeter.voltage(voltageL1, L1_POWER_METER_MODBUS_ID) == SDM120CTPowerMeter::ReadStatus::Ok) {
        data.L1Voltage_x10 = static_cast<uint16_t>(voltageL1 * 10);
    } else {
        data.L1Voltage_x10 = 0;
    }
    delay(60);

    if (sdm120ctPowerMeter.voltage(voltageL2, L2_POWER_METER_MODBUS_ID) == SDM120CTPowerMeter::ReadStatus::Ok) {
        data.L2Voltage_x10 = static_cast<uint16_t>(voltageL2 * 10);
    } else {
        data.L2Voltage_x10 = 0;
    }
}   

void PowerMeterFsm::calculateTotalAndPeriondPowerData(uint32_t l1TotalEnergyWh, uint32_t l2TotalEnergyWh, uint32_t homeTotalEnergyWh, MeasurementData &data) {
   
    if (_l1TotalPower == 0) {
        data.L1Power = 0;
        data.L1TotalPower = l1TotalEnergyWh;
        _l1TotalPower = l1TotalEnergyWh;
    } else {
        data.L1Power = l1TotalEnergyWh - _l1TotalPower;
        _l1TotalPower = l1TotalEnergyWh ;
        data.L1TotalPower = _l1TotalPower;
    }
    if (_l2TotalPower == 0) {
        data.L2Power = 0;
        data.L2TotalPower = l2TotalEnergyWh;
        _l2TotalPower = l2TotalEnergyWh;
    } else {
        data.L2Power = l2TotalEnergyWh - _l2TotalPower;
        _l2TotalPower = l2TotalEnergyWh;
        data.L2TotalPower = _l2TotalPower;
    }

    //TODO logic for home - check it !!
    if (_homeTotalPower == 0) {
        data.HomePower = 0;
        data.HomeTotalPower = homeTotalEnergyWh;
        _homeTotalPower = homeTotalEnergyWh;
    } else {
        data.HomePower = homeTotalEnergyWh - _homeTotalPower;
        _homeTotalPower = homeTotalEnergyWh;
        data.HomeTotalPower = _homeTotalPower;
    }


    LOG_INFO("Powers - L1: %u Wh, L2: %u Wh, Home: %u Wh | Totals - L1: %u Wh, L2: %u Wh, Home: %u Wh", 
             data.L1Power, data.L2Power, data.HomePower, 
             data.L1TotalPower, data.L2TotalPower, data.HomeTotalPower);
}


void PowerMeterFsm::resetCounters() {
    _retryCounter = 0;
    _l1ReadErrorCount = 0;
    _l2ReadErrorCount = 0;
}

