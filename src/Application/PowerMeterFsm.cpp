#include "PowerMeterFsm.h"  
#include "GlobalTypes.h"

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
    delay(60); 
    auto statusL2 = sdm120ctPowerMeter.importActiveEnergy(l2Energy, L2_POWER_METER_MODBUS_ID);
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

    const float homeTotalEnergyRaw = orwe520PowerMeter.totalEnergyKWh();
    homeTotalEnergy = homeTotalEnergyRaw;

    Serial.print("L1 Energy: ");
    Serial.print(l1Energy); 
    Serial.print(" kWh, L2 Energy: ");
    Serial.print(l2Energy);
    Serial.print(" kWh, Home Total Energy: ");
    Serial.print(homeTotalEnergyRaw);
    Serial.println(" kWh");

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

void PowerMeterFsm::calculateTotalAndPeriondPowerData(float l1Energy, float l2Energy, float homeTotalEnergy, MeasurementData &data) {
    constexpr float kWhToWh = 1000.0f;
    const uint32_t l1EnergyWh = static_cast<uint32_t>(l1Energy * kWhToWh);
    const uint32_t l2EnergyWh = static_cast<uint32_t>(l2Energy * kWhToWh);
    const uint32_t homeTotalEnergyWh = static_cast<uint32_t>(homeTotalEnergy * kWhToWh);

    data.L1Power = l1EnergyWh > _l1TotalPower ? l1EnergyWh - _l1TotalPower : 0;
    data.L2Power = l2EnergyWh > _l2TotalPower ? l2EnergyWh - _l2TotalPower : 0;
    data.HomePower = homeTotalEnergyWh > _homeTotalPower ? homeTotalEnergyWh - _homeTotalPower : 0;

    _l1TotalPower = l1EnergyWh;
    _l2TotalPower = l2EnergyWh;
    _homeTotalPower = homeTotalEnergyWh;
    data.L1TotalPower = _l1TotalPower;
    data.L2TotalPower = _l2TotalPower;  
    data.HomeTotalPower = _homeTotalPower;
}


void PowerMeterFsm::resetCounters() {
    _retryCounter = 0;
    _l1ReadErrorCount = 0;
    _l2ReadErrorCount = 0;
}

