#include "MqttMeasurementsPublisher.h"
#include "Utils/TimeUtils.h"

bool MqttMeasurementsPublisher::publishPacket(const MeasurementDataPacket& m) {
    bool ok = true;

    // Publish timestamp in ISO 8601 format
    _client.loop();
    delay(60);
    //ok &= publishTopicPayload("timestamp", formatIsoTimestamp(m.timestamp).c_str());
    //_client.loop();

    ok &= publishUint("power/l1", m.L1Power);
    ok &= publishUint("power/l2", m.L2Power);
    //_client.loop();

    ok &= publishUint("power/heater", m.HeaterPower);
    ok &= publishUint("power/home_total", m.HomeTotalPower);
    //_client.loop();

    ok &= publishTopicPayload("voltage/l1", formatVoltageString(m.L1Voltage_x10).c_str());
    ok &= publishTopicPayload("voltage/l2", formatVoltageString(m.L2Voltage_x10).c_str());
    //_client.loop();
    ok &= publishUint("heater/enable_seconds", m.HeaterEnableForSeconds);
    _client.loop();

    Serial.print("MQTT publish: success=");
    Serial.print(_publishSuccessCount);
    Serial.print(", fail=");    
    Serial.println(_punlishFailureCount);
    _publishSuccessCount = 0;
    _punlishFailureCount = 0;
    return ok;
}

bool MqttMeasurementsPublisher::publishOnlineHeartbeat(uint32_t epoch) {
    // Build payload: "online <ISO8601>"
    _client.loop();
    delay(60);
    StaticString32 ts = formatIsoTimestamp(epoch);

    Serial.print("Publishing online heartbeat with timestamp: ");
    Serial.println(ts.c_str()); 
    Serial.print("_baseTopic: ");
    Serial.println(_baseTopic);

    // Topic: <baseTopic>/online (baseTopic provided in ctor, e.g. lacko/shl_c1/status)
    return publishTopicPayload("online", ts.c_str());
}

StaticString32 MqttMeasurementsPublisher::formatVoltageString(uint16_t value_x10) const {
    char payload[16];
    unsigned long whole = value_x10 / 10UL;
    unsigned long frac = value_x10 % 10UL;
    snprintf(payload, sizeof(payload), "%lu.%lu", whole, frac);
    return StaticString32(payload);
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

bool MqttMeasurementsPublisher::publishUint(const char* name, const char* value) {
    return publishTopicPayload(name, value);
}

bool MqttMeasurementsPublisher::publishTopicPayload(const char* fieldName, const char* payload) {
        StaticString192 topic;
        topic.append(_baseTopic);
        topic.append("/");
        topic.append(fieldName);

        _client.loop();
        const char* t = topic.c_str();
        const size_t tlen = strlen(t);
        const size_t plen = strlen(payload);

        bool ok = false;
        for (uint8_t attempt = 0; attempt < 2; ++attempt) { // 1 szybka ponowna próba
            _client.loop();
            ok = _client.publish(t, payload);
            if (ok) break;
            delay(60);               // krótki backoff żeby opróżnić bufor TCP
            Serial.println("retry...");
            _client.loop();
        }

        // utrzymaj sesję i daj czas na wysyłkę kolejki
        _client.loop();
        delay(40);

        if (ok) {
            _publishSuccessCount++;
        } else {
            _punlishFailureCount++;
            Serial.printf("MQTT publish FAILED: topic='%s' topic_len=%u payload_len=%u connected=%d state=%d\n",
                          t, (unsigned)tlen, (unsigned)plen, _client.connected(), _client.state());
        }
        return ok;
}
