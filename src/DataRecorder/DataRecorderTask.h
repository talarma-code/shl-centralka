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
        // File where events are stored (one JSON object per line)
    static constexpr const char* kEventsFileName = "events.json";
    static constexpr const char* kLogsFileName = "logs.json";
};