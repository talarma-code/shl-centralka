#include "DataRecorderTask.h"
#include <time.h>
#include "SimpleJsonParser.h"
#include <Utils/TimeUtils.h>

DataRecorderTask::DataRecorderTask() : ActiveTask("DataRecorderTask", 8192, 3, 1),
    dataRecorderQueue(10), logRecorderQueue(20)
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
        sdRecorder.createFile(SdRecorder::FileType::Logs, DataRecorderTask::kLogsFileName);
    }
}

void DataRecorderTask::loop()
{
    SystemLogPacket logPacket;
    // Drain all pending log messages; between each give time back to scheduler
    while (logRecorderQueue.receive(logPacket, 0))
    {
        StaticString192 logLine;
        logLine.snprintf("[%s] [%u] %s", formatIsoTimestamp(logPacket.timestamp).c_str(), static_cast<uint8_t>(logPacket.level), logPacket.logMessage.c_str());
        if (!sdRecorder.append(SdRecorder::FileType::Logs, logLine)) {
            Serial.println("Failed to write log to SD");
        } else {
            //TODO: remove debug print
            Serial.println("Log saved to SD: ");
            Serial.println(logLine.c_str());
        }

        // Allow other tasks to run between processing log entries
        vTaskDelay(10);
    }
    SystemMessagePacket packet;
    if (dataRecorderQueue.receive(packet, 100))
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
    resetWatchdog();
}

QueueHandle_t DataRecorderTask::logsQueue() {
    return logRecorderQueue.nativeHandle();
}

QueueHandle_t DataRecorderTask::recordQueue() {
    return dataRecorderQueue.nativeHandle();
}

