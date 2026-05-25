#include "MqttMeasurementsPublisher.h"
#include "TimeUtils.h"
#include "ResetReason.h"
#include "Log.h"

bool MqttMeasurementsPublisher::publishPacket(const MeasurementDataPacket& m) {
    // Reset counters for this packet so that we can decide
    _publishSuccessCount = 0;
    _punlishFailureCount = 0;
    _client.loop();         //this is cricial to keep connection alive and avoid publish failures
    delay(60);              //this is cricial to keep connection alive and avoid publish failures

    publishUint("power/l1", m.L1Power);
    publishUint("power/l2", m.L2Power);
    publishUint("power/heater", m.HeaterPower);
    publishUint("power/home_total", m.HomeTotalPower);

    publishTopicPayload("voltage/l1", formatVoltageString(m.L1Voltage_x10).c_str());
    publishTopicPayload("voltage/l2", formatVoltageString(m.L2Voltage_x10).c_str());
    publishUint("heater/enable_seconds", m.HeaterEnableForSeconds);

    _client.loop();

    LOG_DEBUG("MQTT publish: success=%d, fail=%d", _publishSuccessCount, _punlishFailureCount);

    // If 3 or more individual publishes in this packet failed,
    // treat the whole packet as failed so caller can trigger reconnect.
    if (_punlishFailureCount >= 3) {
        return false;
    }
    return true;
}

bool MqttMeasurementsPublisher::publishResetReason() {
    _publishSuccessCount = 0;
    _punlishFailureCount = 0;
    _client.loop();         //this is cricial to keep connection alive and avoid publish failures
    delay(60);              //this is cricial to keep connection alive and avoid publish failures

    publishTopicPayload("reason", lastResetReason());
    publishUint("code", lastResetReasonCode());

    _client.loop();
    LOG_DEBUG("MQTT publish: success=%d, fail=%d", _publishSuccessCount, _punlishFailureCount);

    // If 3 or more individual publishes in this packet failed,
    // treat the whole packet as failed so caller can trigger reconnect.
    if (_punlishFailureCount >= 3) {
        return false;
    }
    return true;
}

bool MqttMeasurementsPublisher::publishOnlineHeartbeat(uint32_t epoch) {
    _client.loop();  //this is cricial to keep connection alive and avoid publish failures
    delay(60);       //this is cricial to keep connection alive and avoid publish failures
    StaticString32 ts = formatIsoTimestamp(epoch);

    LOG_DEBUG("Publishing online heartbeat with timestamp: %s", ts.c_str());
    return publishTopicPayload("online", ts.c_str());
}

StaticString32 MqttMeasurementsPublisher::formatVoltageString(uint16_t value_x10) const {
    char payload[16];
    unsigned long whole = value_x10 / 10UL;
    unsigned long frac = value_x10 % 10UL;
    snprintf(payload, sizeof(payload), "%lu.%lu", whole, frac);
    return StaticString32(payload);
}

void MqttMeasurementsPublisher::publishInt(const char* name, int32_t value) {
    char payload[24];
    snprintf(payload, sizeof(payload), "%ld", (long)value);
    publishTopicPayload(name, payload);
}

void MqttMeasurementsPublisher::publishUint(const char* name, uint32_t value) {
    char payload[24];
    snprintf(payload, sizeof(payload), "%lu", (unsigned long)value);
    publishTopicPayload(name, payload);
}

void MqttMeasurementsPublisher::publishUint(const char* name, const char* value) {
    publishTopicPayload(name, value);
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
            LOG_DEBUG("publish retry...");
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
