#include "PowerMeterFsm.h"  
#include "GlobalTypes.h"

#define L1_POWER_METER_MODBUS_ID 1
#define L2_POWER_METER_MODBUS_ID 2

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
        return {Next::Error, 0};
    }

}
 
PowerMeterFsm::Result PowerMeterFsm::measurmentState(MeasurementData &data) {
    float l1Energy, l2Energy, homeTotalEnergy;
    if (getPowerData(l1Energy, l2Energy, homeTotalEnergy)) {
        calculateTotalAndPeriondPowerData(l1Energy, l2Energy, homeTotalEnergy, data);
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

bool PowerMeterFsm::getPowerData(float& l1Energy, float& l2Energy, float& homeTotalEnergy) {
    auto statusL1 = sdm120ctPowerMeter.importActiveEnergy(l1Energy, L1_POWER_METER_MODBUS_ID);
    auto statusL2 = sdm120ctPowerMeter.importActiveEnergy(l2Energy, L2_POWER_METER_MODBUS_ID);

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

    homeTotalEnergy = static_cast<uint32_t>(orwe520PowerMeter.totalEnergyKWh());
    l1Energy = static_cast<uint32_t>(l1Energy);
    l2Energy = static_cast<uint32_t>(l2Energy);
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

    if (sdm120ctPowerMeter.voltage(voltageL2, L2_POWER_METER_MODBUS_ID) == SDM120CTPowerMeter::ReadStatus::Ok) {
        data.L2Voltage_x10 = static_cast<uint16_t>(voltageL2 * 10);
    } else {
        data.L2Voltage_x10 = 0;
    }
}   

void PowerMeterFsm::calculateTotalAndPeriondPowerData(float l1Energy, float l2Energy, float homeTotalEnergy, MeasurementData &data) {

    data.L1Power = _l1TotalPower > (uint32_t)(l1Energy) ? _l1TotalPower - (uint32_t)(l1Energy) : (uint32_t)(l1Energy);
    data.L2Power = _l2TotalPower > (uint32_t)(l2Energy) ? _l2TotalPower - (uint32_t)(l2Energy) : (uint32_t)(l2Energy);
    data.HomePower = _homeTotalPower > (uint32_t)(homeTotalEnergy) ? _homeTotalPower - (uint32_t)(homeTotalEnergy) : (uint32_t)(homeTotalEnergy);

    _l1TotalPower = (uint32_t)(l1Energy);
    _l2TotalPower = (uint32_t)(l2Energy);
    _homeTotalPower = (uint32_t)(homeTotalEnergy);
    data.L1TotalPower = _l1TotalPower;
    data.L2TotalPower = _l2TotalPower;  
    data.HomeTotalPower = _homeTotalPower;
}


void PowerMeterFsm::resetCounters() {
    _retryCounter = 0;
    _l1ReadErrorCount = 0;
    _l2ReadErrorCount = 0;
}

