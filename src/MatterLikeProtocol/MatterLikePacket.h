#pragma once

#include <Arduino.h>

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