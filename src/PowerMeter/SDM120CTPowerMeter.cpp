#include "SDM120CTPowerMeter.h"

#define MODBUS_ESP32_MASTER_ID 1

// SDM120CT Modbus Register Addresses
#define SDM120_VOLTAGE 0x0000
#define SDM120_ELECTRIC_CURRENT 0x0006
#define SDM120_ACTIVE_POWER 0x000C
#define SDM120_APPARENT_POWER 0x0012
#define SDM120_REACTIVE_POWER 0x0018
#define SDM120_POWER_FACTOR 0x001E
#define SDM120_FREQUENCY 0x0046
#define SDM120_IMPORT_ACTIVE_ENERGY 0x0048
#define SDM120_EXPORT_ACTIVE_ENERGY 0x004A
#define SDM120_IMPORT_REACTIVE_ENERGY 0x004C
#define SDM120_EXPORT_REACTIVE_ENERGY 0x004E
#define SDM120_TOTAL_ACTIVE_ENERGY 0x0156
#define SDM120_TOTAL_REACTIVE_ENERGY 0x0158


void SDM120CTPowerMeter::setup() {
    // Baud rate: 9600, Parity: None, RX2 = 33(GPIO33), TX2 = 32(GPIO32)
    Serial1.begin(9600, SERIAL_8N1, 33, 32);
}

// Compute Modbus CRC16 (A001 polynomial)
static uint16_t modbusCRC(const uint8_t *buf, size_t len) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; ++i) {
        crc ^= buf[i];
        for (int j = 0; j < 8; ++j) {
            if (crc & 0x0001) crc = (crc >> 1) ^ 0xA001;
            else crc >>= 1;
        }
    }
    return crc;
}

void SDM120CTPowerMeter::debugPrintRequest(uint8_t slave, uint8_t func, uint16_t startAddr, uint16_t count) {
    uint8_t frame[8];
    frame[0] = slave;
    frame[1] = func;
    frame[2] = (startAddr >> 8) & 0xFF;
    frame[3] = startAddr & 0xFF;
    frame[4] = (count >> 8) & 0xFF;
    frame[5] = count & 0xFF;
    uint16_t crc = modbusCRC(frame, 6);
    uint8_t crc_lo = crc & 0xFF;
    uint8_t crc_hi = (crc >> 8) & 0xFF;

    // Print human-friendly hex frame
    Serial.print("Modbus TX: ");
    for (int i = 0; i < 6; ++i) {
        if (frame[i] < 0x10) Serial.print('0');
        Serial.print(frame[i], HEX);
        Serial.print(' ');
    }
    if (crc_lo < 0x10) Serial.print('0'); Serial.print(crc_lo, HEX); Serial.print(' ');
    if (crc_hi < 0x10) Serial.print('0'); Serial.print(crc_hi, HEX);
    Serial.println();
}

SDM120CTPowerMeter::ReadStatus SDM120CTPowerMeter::voltage(float &value, uint8_t slaveId) {
    node.begin(slaveId, Serial1);
    uint8_t result;
    debugPrintRequest(slaveId, 0x04, SDM120_VOLTAGE, 2);
    result = node.readInputRegisters(SDM120_VOLTAGE, 2); // 2 registers = 4 bytes

    if (result == node.ku8MBSuccess) {
        value = getFloatValue();
        return ReadStatus::Ok;
    } else {
        modbusError(result);
        value = 0.0f;
        return ReadStatus::Error;
    }
}

SDM120CTPowerMeter::ReadStatus SDM120CTPowerMeter::electricCurrent(float &value, uint8_t slaveId) {
    node.begin(slaveId, Serial1);
    uint8_t result;
    debugPrintRequest(slaveId, 0x04, SDM120_ELECTRIC_CURRENT, 2);
    result = node.readInputRegisters(SDM120_ELECTRIC_CURRENT, 2); // 2 registers = 4 bytes

    if (result == node.ku8MBSuccess) {
        value = getFloatValue();
        return ReadStatus::Ok;
    } else {
        modbusError(result);
        value = 0.0f;
        return ReadStatus::Error;
    }
}

SDM120CTPowerMeter::ReadStatus SDM120CTPowerMeter::activePower(float &value, uint8_t slaveId) {
    node.begin(slaveId, Serial1);
    uint8_t result;
    debugPrintRequest(slaveId, 0x04, SDM120_ACTIVE_POWER, 2);
    result = node.readInputRegisters(SDM120_ACTIVE_POWER, 2); // 2 registers = 4 bytes

    if (result == node.ku8MBSuccess) {
        value = getFloatValue();
        return ReadStatus::Ok;
    } else {
        modbusError(result);
        value = 0.0f;
        return ReadStatus::Error;
    }
}

SDM120CTPowerMeter::ReadStatus SDM120CTPowerMeter::apparentPower(float &value, uint8_t slaveId) {
    node.begin(slaveId, Serial1);
    uint8_t result;
    debugPrintRequest(slaveId, 0x04, SDM120_APPARENT_POWER, 2);
    result = node.readInputRegisters(SDM120_APPARENT_POWER, 2); // 2 registers = 4 bytes

    if (result == node.ku8MBSuccess) {
        value = getFloatValue();
        return ReadStatus::Ok;
    } else {
        modbusError(result);
        value = 0.0f;
        return ReadStatus::Error;
    }
}

SDM120CTPowerMeter::ReadStatus SDM120CTPowerMeter::reactivePower(float &value, uint8_t slaveId) {
    node.begin(slaveId, Serial1);
    uint8_t result;
    debugPrintRequest(slaveId, 0x04, SDM120_REACTIVE_POWER, 2);
    result = node.readInputRegisters(SDM120_REACTIVE_POWER, 2); // 2 registers = 4 bytes

    if (result == node.ku8MBSuccess) {
        value = getFloatValue();
        return ReadStatus::Ok;
    } else {
        modbusError(result);
        value = 0.0f;
        return ReadStatus::Error;
    }
}

SDM120CTPowerMeter::ReadStatus SDM120CTPowerMeter::powerFactor(float &value, uint8_t slaveId) {
    node.begin(slaveId, Serial1);
    uint8_t result;
    debugPrintRequest(slaveId, 0x04, SDM120_POWER_FACTOR, 2);
    result = node.readInputRegisters(SDM120_POWER_FACTOR, 2); // 2 registers = 4 bytes

    if (result == node.ku8MBSuccess) {
        value = getFloatValue();
        return ReadStatus::Ok;
    } else {
        modbusError(result);
        value = 0.0f;
        return ReadStatus::Error;
    }
}

SDM120CTPowerMeter::ReadStatus SDM120CTPowerMeter::frequency(float &value, uint8_t slaveId) {
    node.begin(slaveId, Serial1);
    uint8_t result;
    debugPrintRequest(slaveId, 0x04, SDM120_FREQUENCY, 2);
    result = node.readInputRegisters(SDM120_FREQUENCY, 2); // 2 registers = 4 bytes

    if (result == node.ku8MBSuccess) {
        value = getFloatValue();
        return ReadStatus::Ok;
    } else {
        modbusError(result);
        value = 0.0f;
        return ReadStatus::Error;
    }
}

SDM120CTPowerMeter::ReadStatus SDM120CTPowerMeter::importActiveEnergy(float &value, uint8_t slaveId) {
    node.begin(slaveId, Serial1);
    uint8_t result;
    debugPrintRequest(slaveId, 0x04, SDM120_IMPORT_ACTIVE_ENERGY, 2);
    result = node.readInputRegisters(SDM120_IMPORT_ACTIVE_ENERGY, 2); // 2 registers = 4 bytes

    if (result == node.ku8MBSuccess) {
        value = getFloatValue();
        return ReadStatus::Ok;
    } else {
        modbusError(result);
        value = 0.0f;
        return ReadStatus::Error;
    }
}

SDM120CTPowerMeter::ReadStatus SDM120CTPowerMeter::exportActiveEnergy(float &value, uint8_t slaveId) {
    node.begin(slaveId, Serial1);
    uint8_t result;
    debugPrintRequest(slaveId, 0x04, SDM120_EXPORT_ACTIVE_ENERGY, 2);
    result = node.readInputRegisters(SDM120_EXPORT_ACTIVE_ENERGY, 2); // 2 registers = 4 bytes

    if (result == node.ku8MBSuccess) {
        value = getFloatValue();
        return ReadStatus::Ok;
    } else {
        modbusError(result);
        value = 0.0f;
        return ReadStatus::Error;
    }
}

SDM120CTPowerMeter::ReadStatus SDM120CTPowerMeter::importReactiveEnergy(float &value, uint8_t slaveId) {
    node.begin(slaveId, Serial1);
    uint8_t result;
    debugPrintRequest(slaveId, 0x04, SDM120_IMPORT_REACTIVE_ENERGY, 2);
    result = node.readInputRegisters(SDM120_IMPORT_REACTIVE_ENERGY, 2); // 2 registers = 4 bytes

    if (result == node.ku8MBSuccess) {
        value = getFloatValue();
        return ReadStatus::Ok;
    } else {
        modbusError(result);
        value = 0.0f;
        return ReadStatus::Error;
    }
}

SDM120CTPowerMeter::ReadStatus SDM120CTPowerMeter::exportReactiveEnergy(float &value, uint8_t slaveId) {
    node.begin(slaveId, Serial1);
    uint8_t result;
    debugPrintRequest(slaveId, 0x04, SDM120_EXPORT_REACTIVE_ENERGY, 2);
    result = node.readInputRegisters(SDM120_EXPORT_REACTIVE_ENERGY, 2); // 2 registers = 4 bytes

    if (result == node.ku8MBSuccess) {
        value = getFloatValue();
        return ReadStatus::Ok;
    } else {
        modbusError(result);
        value = 0.0f;
        return ReadStatus::Error;
    }
}

SDM120CTPowerMeter::ReadStatus SDM120CTPowerMeter::totalActiveEnergy(float &value, uint8_t slaveId) {
    node.begin(slaveId, Serial1);
    uint8_t result;
    debugPrintRequest(slaveId, 0x04, SDM120_TOTAL_ACTIVE_ENERGY, 2);
    result = node.readInputRegisters(SDM120_TOTAL_ACTIVE_ENERGY, 2); // 2 registers = 4 bytes
    if (result == node.ku8MBSuccess) {
        value = getFloatValue();
        return ReadStatus::Ok;
    } else {
        modbusError(result);
        value = 0.0f;
        return ReadStatus::Error;
    }
}

SDM120CTPowerMeter::ReadStatus SDM120CTPowerMeter::totalReactiveEnergy(float &value, uint8_t slaveId) {
    node.begin(slaveId, Serial1);
    uint8_t result;
    debugPrintRequest(slaveId, 0x04, SDM120_TOTAL_REACTIVE_ENERGY, 2);
    result = node.readInputRegisters(SDM120_TOTAL_REACTIVE_ENERGY, 2); // 2 registers = 4 bytes

    if (result == node.ku8MBSuccess) {
        value = getFloatValue();
        return ReadStatus::Ok;
    } else {
        modbusError(result);
        value = 0.0f;
        return ReadStatus::Error;
    }
}

float SDM120CTPowerMeter::getFloatValue() {
    uint16_t hi = node.getResponseBuffer(0);
    uint16_t lo = node.getResponseBuffer(1);

    float value;
    ((uint16_t*)&value)[1] = hi;
    ((uint16_t*)&value)[0] = lo;
    return value;
}

void SDM120CTPowerMeter::modbusError(uint8_t result) {
    Serial.print("Error: 0x");
    Serial.println(result, HEX);

    switch (result) {
        case 0x01: Serial.println("Illegal Function"); break;
        case 0x02: Serial.println("Illegal Data Address"); break;
        case 0x03: Serial.println("Illegal Data Value"); break;
        case 0x0B: Serial.println("Timeout / No Response"); break;
        default:
            Serial.println("Unknown error");
    }
}
