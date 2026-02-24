#include "PowerMeterFsm.h"  

#define L1_POWER_METER_MODBUS_ID 1
#define L2_POWER_METER_MODBUS_ID 2

PowerMeterFsm::PowerMeterFsm() : orwe520PowerMeter(), sdm120ctPowerMeter() {}

void PowerMeterFsm::setup() {
    orwe520PowerMeter.setup();
    sdm120ctPowerMeter.setup();
}

PowerMeterFsm::Status PowerMeterFsm::messurementReady(MeasurementData &data) {
    float l1Energy, l2Energy, homeTotalEnergy;
    if (getPowerData(l1Energy, l2Energy, homeTotalEnergy)) {
        calculateTotalAndPeriondPowerData(l1Energy, l2Energy, homeTotalEnergy, data);
        getVoltage(data);
        _retryCounter = 0; // reset retry counter on successful measurement 
        return Status::MesurmentOk;

    } else {
        _retryCounter++;
        if (_retryCounter > 3) {
            _retryCounter = 0;
            return Status::Error;
        }
        return Status::RetryMesurment;
    }
}

bool PowerMeterFsm::getPowerData(float& l1Energy, float& l2Energy, float& homeTotalEnergy) {
    auto statusL1 = sdm120ctPowerMeter.importActiveEnergy(l1Energy, L1_POWER_METER_MODBUS_ID);
    auto statusL2 = sdm120ctPowerMeter.importActiveEnergy(l2Energy, L2_POWER_METER_MODBUS_ID);

    if (statusL1 != SDM120CTPowerMeter::ReadStatus::Ok ||
        statusL2 != SDM120CTPowerMeter::ReadStatus::Ok) {
        return false;
    }

    homeTotalEnergy = static_cast<uint32_t>(orwe520PowerMeter.totalEnergyKWh());
    l1Energy = static_cast<uint32_t>(l1Energy);
    l2Energy = static_cast<uint32_t>(l2Energy);
    return true;
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

