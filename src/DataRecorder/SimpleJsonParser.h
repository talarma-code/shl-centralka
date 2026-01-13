#pragma once

#include "GlobalTypes.h"
#include "IntertaskDataModel.h"

// SimpleJsonParser: lightweight, allocation-free helpers to serialize/deserialize
// Measurement and Event structs used by DataRecorderTask.
// Uses ISO 8601 timestamp format: "YYYY-MM-DDTHH:MM:SS" for timestamps.
class SimpleJsonParser {
public:
    // ISO timestamp helpers are implemented in Utils/TimeUtils and wrapped here
    static StaticString32 formatIsoTimestamp(uint32_t epoch);
    static bool parseIsoTimestamp(const char* str, uint32_t &outEpoch);

    // Measurement serialization/deserialization
    static StaticString192 serializeMeasurement(const MeasurementDataPacket& m);
    static bool deserializeMeasurement(const StaticString192& json, MeasurementDataPacket& out);

    // Event serialization/deserialization (includes mapping reason strings)
    static StaticString192 serializeEvent(const SystemEventPacket& e);
    static bool deserializeEvent(const StaticString192& json, SystemEventPacket& out);
};
