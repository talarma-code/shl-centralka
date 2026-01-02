#include "DataRecorderTask.h"

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
        sdRecorder.createFile(DataRecorderTask::kMeasurementsFileName);
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
            StaticString128 json = serializeMeasurement(packet.payload.measurementData);
            if (!sdRecorder.append(json.c_str())) {
                Serial.println("Failed to write measurement to SD");
            } else {
                Serial.println("Measurement saved to SD: ");
                Serial.println(json.c_str());
            }
        }
        else if (packet.type == SystemDataType::Events)
        {
            // (optional) handle events later
        }
        
    }
    
}

// -------------------- Serialization helpers --------------------
StaticString128 DataRecorderTask::serializeMeasurement(const MeasurementDataPacket& m) const {
    char buf[128];
    int n = snprintf(buf, sizeof(buf),
        "{\"timestamp\":%u,\"L1Power\":%u,\"L2Power\":%u,\"HeaterPower\":%u,\"HomeTotalPower\":%u,\"L1Voltage_x10\":%u,\"L2Voltage_x10\":%u}",
        (unsigned)m.timestamp,
        (unsigned)m.L1Power,
        (unsigned)m.L2Power,
        (unsigned)m.HeaterPower,
        (unsigned)m.HomeTotalPower,
        (unsigned)m.L1Voltage_x10,
        (unsigned)m.L2Voltage_x10);

    StaticString128 s;
    if (n > 0) {
        s.assign(buf);
    } else {
        s.clear();
    }
    return s;
}

bool DataRecorderTask::deserializeMeasurement(const StaticString128& json, MeasurementDataPacket& out) const {
    const char* j = json.c_str();

    auto extractUInt = [&](const char* key)->uint32_t {
        const char* p = strstr(j, key);
        if (!p) return 0;
        p = strchr(p, ':');
        if (!p) return 0;
        p++;
        while (*p && (*p == ' ' || *p == '"')) p++;
        char* endptr;
        uint32_t val = (uint32_t) strtoul(p, &endptr, 10);
        return val;
    };

    out.timestamp = extractUInt("\"timestamp\"");
    out.L1Power = extractUInt("\"L1Power\"");
    out.L2Power = extractUInt("\"L2Power\"");
    out.HeaterPower = extractUInt("\"HeaterPower\"");
    out.HomeTotalPower = extractUInt("\"HomeTotalPower\"");
    out.L1Voltage_x10 = (uint16_t)extractUInt("\"L1Voltage_x10\"");
    out.L2Voltage_x10 = (uint16_t)extractUInt("\"L2Voltage_x10\"");

    return out.timestamp != 0;
}

const char* 
DataRecorderTask::resetReasonToString(esp_reset_reason_t reason) const {
    switch (reason) {
        case ESP_RST_UNKNOWN:       return "Unknown";
        case ESP_RST_POWERON:       return "Power On Reset";
        case ESP_RST_EXT:           return "External Reset";
        case ESP_RST_SW:            return "Software Reset";
        case ESP_RST_PANIC:         return "Panic Reset";
        case ESP_RST_INT_WDT:       return "Interrupt Watchdog Reset";
        case ESP_RST_TASK_WDT:      return "Task Watchdog Reset";
        case ESP_RST_WDT:           return "Other Watchdog Reset";
        case ESP_RST_DEEPSLEEP:     return "Deep Sleep Reset";
        case ESP_RST_BROWNOUT:      return "Brownout Reset";
        case ESP_RST_SDIO:          return "SDIO Reset";
        case ESP_RST_USB:           return "USB Reset";
        case ESP_RST_JTAG:          return "JTAG Reset";
        case ESP_RST_EFUSE:         return "Efuse Reset";
        case ESP_RST_PWR_GLITCH:    return "Power Glitch Reset";
        case ESP_RST_CPU_LOCKUP:    return "CPU Lockup Reset";
        default:                    return "Invalid Reason";
    }
}