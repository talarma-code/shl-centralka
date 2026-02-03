#include "ORWE504PowerMeter.h"


#define MODBUS_ESP32_MASTER_ID 1

// OR-WE-504 Modbus register addresses (decimal 0–7 from documentation)
#define ORWE504_VOLTAGE             0x0000  // 00 - Voltage
#define ORWE504_ELECTRIC_CURRENT    0x0001  // 01 - Current
#define ORWE504_FREQUENCY           0x0002  // 02 - Frequency
#define ORWE504_ACTIVE_POWER        0x0003  // 03 - Active Power
#define ORWE504_REACTIVE_POWER      0x0004  // 04 - Reactive Power
#define ORWE504_APPARENT_POWER      0x0005  // 05 - Apparent Power
#define ORWE504_POWER_FACTOR        0x0006  // 06 - Power factor
#define ORWE504_TOTAL_ACTIVE_POWERD 0x0007  // 07 - Active energy (2 registers)


void ORWE504PowerMeter::setup() {
    //for hardware configuration: connect Tx from ESP32 board to TX on Rs485 converter board, the same for RX
    Serial2.begin(9600, SERIAL_8N1, 32, 33); // RX2 = 32(GPIO32), TX2 = 33(GPIO33)
}

float ORWE504PowerMeter::voltage(uint8_t slaveId) {
  node.begin(slaveId, Serial2);
  uint8_t result;
  result = node.readInputRegisters(ORWE504_VOLTAGE, 1); // 1 rejestr

  if (result == node.ku8MBSuccess) {
    return getRegister16Value();

  } else {
    modbusError(result);
    return 0;
  }
}


float ORWE504PowerMeter::electricCurrent(uint8_t slaveId) {
  node.begin(slaveId, Serial2);
  uint8_t result;
  result = node.readInputRegisters(ORWE504_ELECTRIC_CURRENT, 1); // 1 rejestr

  if (result == node.ku8MBSuccess) {
    return getRegister16Value();

  } else {
    modbusError(result);
    return 0;
  }
}


float ORWE504PowerMeter::activePower(uint8_t slaveId) {
  node.begin(slaveId, Serial2);
  uint8_t result;
  result = node.readInputRegisters(ORWE504_ACTIVE_POWER, 1); // 1 rejestr

  if (result == node.ku8MBSuccess) {
    return getRegister16Value();

  } else {
    modbusError(result);
    return 0;
  }
}

float ORWE504PowerMeter::reactivePower(uint8_t slaveId) {
  node.begin(slaveId, Serial2);
  uint8_t result;
  result = node.readInputRegisters(ORWE504_REACTIVE_POWER, 1); // 1 rejestr

  if (result == node.ku8MBSuccess) {
    return getRegister16Value();

  } else {
    modbusError(result);
    return 0;
  }
}

float ORWE504PowerMeter::apparentPower(uint8_t slaveId) {
  node.begin(slaveId, Serial2);
  uint8_t result;
  result = node.readInputRegisters(ORWE504_APPARENT_POWER, 1); // 1 rejestr

  if (result == node.ku8MBSuccess) {
    return getRegister16Value();

  } else {
    modbusError(result);
    return 0;
  }
}

float ORWE504PowerMeter::powerFactor(uint8_t slaveId) {
  node.begin(slaveId, Serial2);
  uint8_t result;
  result = node.readInputRegisters(ORWE504_POWER_FACTOR, 1); // 1 rejestr

  if (result == node.ku8MBSuccess) {
    return getRegister16Value();

  } else {
    modbusError(result);
    return 0;
  }
}

float ORWE504PowerMeter::frequency(uint8_t slaveId) {
  node.begin(slaveId, Serial2);
  uint8_t result;
  result = node.readInputRegisters(ORWE504_FREQUENCY, 1); // 1 rejestr

  if (result == node.ku8MBSuccess) {
    return getRegister16Value();

  } else {
    modbusError(result);
    return 0;
  }
}

float ORWE504PowerMeter::totalActivePower(uint8_t slaveId) {
  node.begin(slaveId, Serial2);
  uint8_t result;
  result = node.readInputRegisters(ORWE504_TOTAL_ACTIVE_POWERD, 2); // 2 rejestry = 4 bajty

  if (result == node.ku8MBSuccess) {
    return getRegister32Value();

  } else {
    modbusError(result);
    return 0;
  }
}

 float ORWE504PowerMeter::getRegister16Value() {
   uint16_t reg = node.getResponseBuffer(0);
   return static_cast<float>(reg);
 }

 float ORWE504PowerMeter::getRegister32Value() {
   uint16_t hi = node.getResponseBuffer(0);
   uint16_t lo = node.getResponseBuffer(1);
   uint32_t value = (static_cast<uint32_t>(hi) << 16) | lo;
   return static_cast<float>(value);
 }

void ORWE504PowerMeter::modbusError(uint8_t result) {
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

