#include "DDS661PowerMeter.h"


#define MODBUS_ESP32_MASTER_ID 1

#define DDS661_VOLTAGE 0x0000
#define DDS661_ELECTRIC_CURRENT 0x0008
#define DDS661_ACTIVE_POWER 0x0012
#define DDS661_POWER_FACTOR 0x002A
#define DDS661_FREQUENCY 0x0036
#define DDS661_TOTAL_ACTIVE_POWERD 0x0100


void DDS661PowerMeter::setup() {
    //for hardware configuration: connect Tx from ESP32 board to TX on Rs485 converter board, the same for TX
   // Serial2.begin(9600, SERIAL_8E1, 17, 16); // RX2 = 17(GPIO17), TX2 = 16(GPIO16) - orginal code from heatr 
    Serial2.begin(9600, SERIAL_8E1, 33, 32); 
}

float DDS661PowerMeter::voltage(uint8_t slaveId) {
  node.begin(slaveId, Serial2);
  uint8_t result;
  result = node.readInputRegisters(DDS661_VOLTAGE, 2); // 2 rejestry = 4 bajty

  if (result == node.ku8MBSuccess) {
    return getFloatValue();

  } else {
    modbusError(result);
    return 0;
  }
}


float DDS661PowerMeter::electricCurrent(uint8_t slaveId) {
  node.begin(slaveId, Serial2);
  uint8_t result;
  result = node.readInputRegisters(DDS661_ELECTRIC_CURRENT, 2); // 2 rejestry = 4 bajty

  if (result == node.ku8MBSuccess) {
    return getFloatValue();

  } else {
    modbusError(result);
    return 0;
  }
}


float DDS661PowerMeter::activePower(uint8_t slaveId) {
  node.begin(slaveId, Serial2);
  uint8_t result;
  result = node.readInputRegisters(DDS661_ACTIVE_POWER, 2); // 2 rejestry = 4 bajty

  if (result == node.ku8MBSuccess) {
    return getFloatValue();

  } else {
    modbusError(result);
    return 0;
  }
}

float DDS661PowerMeter::frequency(uint8_t slaveId) {
  node.begin(slaveId, Serial2);
  uint8_t result;
  result = node.readInputRegisters(DDS661_FREQUENCY, 2); // 2 rejestry = 4 bajty

  if (result == node.ku8MBSuccess) {
    return getFloatValue();

  } else {
    modbusError(result);
    return 0;
  }
}

float DDS661PowerMeter::totalActivePower(uint8_t slaveId) {
  node.begin(slaveId, Serial2);
  uint8_t result;
  result = node.readInputRegisters(DDS661_TOTAL_ACTIVE_POWERD, 2); // 2 rejestry = 4 bajty

  if (result == node.ku8MBSuccess) {
    return getFloatValue();

  } else {
    modbusError(result);
    return 0;
  }
}

 float DDS661PowerMeter::getFloatValue() {
    uint16_t hi = node.getResponseBuffer(0);
    uint16_t lo = node.getResponseBuffer(1);

    float value;
    ((uint16_t*)&value)[1] = hi;
    ((uint16_t*)&value)[0] = lo;
    return value;
 }

void DDS661PowerMeter::modbusError(uint8_t result) {
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

