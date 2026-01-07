#include "DataRecorderTask.h"
#include <time.h>
#include "SimpleJsonParser.h"

DataRecorderTask::DataRecorderTask() : ActiveTask("DataRecorderTask", 8192, 3, 1),
    dataRecorderQueue(10) 
{
}

void DataRecorderTask::setup()
{
    // Initialize SD and ensure measurements file exists
    if (!sdRecorder.setup()) {
        Serial.println("SdRecorder setup failed");
    } else {
        sdRecorder.createFile(SdRecorder::FileType::Measurements, DataRecorderTask::kMeasurementsFileName);
        sdRecorder.createFile(SdRecorder::FileType::Events, DataRecorderTask::kEventsFileName);
    }
}

void DataRecorderTask::loop()
{
    SystemMessagePacket packet;
    if (dataRecorderQueue.receive(packet, portMAX_DELAY))
    {
        // Save measurement packets as single-line JSON to SD
        if (packet.type == SystemDataType::Measurements)
        {
            StaticString192 json = SimpleJsonParser::serializeMeasurement(packet.payload.measurementData);
            if (!sdRecorder.append(SdRecorder::FileType::Measurements, json)) {
                Serial.println("Failed to write measurement to SD");
            } else {
                Serial.println("Measurement saved to SD: ");
                Serial.println(json.c_str());
            }
        }
        else if (packet.type == SystemDataType::Events)
        {
            StaticString192 json = SimpleJsonParser::serializeEvent(packet.payload.systemEvent);
            if (!sdRecorder.append(SdRecorder::FileType::Events, json)) {
                Serial.println("Failed to write event to SD");
            } else {
                Serial.println("Event saved to SD: ");
                Serial.println(json.c_str());
            }
        }
        
    }
    
}

