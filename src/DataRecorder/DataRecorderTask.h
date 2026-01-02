#pragma once

#include "ActiveQueue.h"
#include "ActiveTask.h"
#include "IntertaskDataModel.h"
#include "SdRecorder.h"
#include "StaticString.h"
#include "GlobalTypes.h"

class DataRecorderTask : public ActiveTask {
public:
    DataRecorderTask(); 
    void setup() override;
    void loop() override;

private:
    ActiveQueue<SystemMessagePacket> dataRecorderQueue;
    SdRecorder sdRecorder;

    // File where measurements are stored (one JSON object per line)
    static constexpr const char* kMeasurementsFileName = "measurements.json";

    // Serialize/deserialize measurement data to/from single-line JSON
    // Use a fixed-capacity StaticString to avoid dynamic allocations
    StaticString128 serializeMeasurement(const MeasurementDataPacket& m) const;
    bool deserializeMeasurement(const StaticString128& json, MeasurementDataPacket& out) const;

    const char* resetReasonToString(esp_reset_reason_t reason) const;
};