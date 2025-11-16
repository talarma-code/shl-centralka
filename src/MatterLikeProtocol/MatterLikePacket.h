#pragma once

#include <Arduino.h>


// Cluster IDs
#define CLUSTER_ONOFF                  0x0006
#define CLUSTER_ELECTRICAL_MEAS        0x0B04

// OnOff Commands
#define CMD_OFF                        0x00
#define CMD_ON                         0x01
#define CMD_TOGGLE                     0x02

// OnOff Attributes (reported state read-back)
#define ATTR_ONOFF_STATE               0x0000

// Electrical Measurement Attributes
#define ATTR_EM_ACTIVE_POWER           0x0505
#define ATTR_EM_RMS_VOLTAGE            0x0508
#define ATTR_EM_RMS_CURRENT            0x050B
#define ATTR_EM_POWER_FACTOR           0x050D

struct MatterLikeHeader {
  uint32_t messageCounter;
  uint16_t sessionId;
  uint8_t flags;
};

struct MatterLikePayload {
  uint32_t fabricId;        // ID sieci / instalacji
  uint16_t nodeId;          // ID urządzenia (gniazdko)
  uint8_t endpointId;       // ID funkcji / endpointu
  uint16_t clusterId;       // typ klastra (OnOff lub ElectricalMeasurement)
  uint16_t attributeId;     // ID atrybutu w klastrze
  uint8_t commandId;        // np. 0x03 = ReadAttribute, 0x04 = ReportAttribute
  int32_t value;            // dla danych liczbowych lub bool (0/1)
};

struct MatterLikePacket {
  MatterLikeHeader header;
  MatterLikePayload payload;
};