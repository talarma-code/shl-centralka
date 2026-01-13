#include "MqttMeasurementsPublisher.h"

bool MqttMeasurementsPublisher::publishPacket(const MeasurementDataPacket& m) {
    bool ok = true;

    ok &= publishUint("timestamp", m.timestamp);

    ok &= publishUint("power/l1", m.L1Power);
    ok &= publishUint("power/l2", m.L2Power);
    ok &= publishUint("power/heater", m.HeaterPower);
    ok &= publishUint("power/home_total", m.HomeTotalPower);

    ok &= publishUint("voltage/l1_x10", m.L1Voltage_x10);
    ok &= publishUint("voltage/l2_x10", m.L2Voltage_x10);

    ok &= publishUint("heater/enable_seconds", m.HeaterEnableForSeconds);
    ok &= publishUint("measurement/type", static_cast<uint32_t>(m.measurementType));

    return ok;
}

bool MqttMeasurementsPublisher::publishInt(const char* name, int32_t value) {
    char payload[24];
    snprintf(payload, sizeof(payload), "%ld", (long)value);
    return publishTopicPayload(name, payload);
}

bool MqttMeasurementsPublisher::publishUint(const char* name, uint32_t value) {
    char payload[24];
    snprintf(payload, sizeof(payload), "%lu", (unsigned long)value);
    return publishTopicPayload(name, payload);
}

bool MqttMeasurementsPublisher::publishTopicPayload(const char* fieldName, const char* payload) {
        StaticString192 topic;
        topic.append(_baseTopic);
        topic.append("/");
        topic.append(fieldName);
        if (_client.publish(topic.c_str(), payload)) {
        return true;
    }
    return _client.publish(topic.c_str(), payload);
}
