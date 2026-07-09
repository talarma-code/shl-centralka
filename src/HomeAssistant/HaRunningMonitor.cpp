#include "HaRunningMonitor.h"
#include "MqttPublisher.h"
#include "TimeUtils.h"
#include "Log.h"

#include <time.h>
#include <Arduino.h>

// Device system clock is maintained in UTC and heartbeat payloads are published as UTC timestamps.
HaRunningMonitor::Result HaRunningMonitor::step() {
    // Maintain MQTT session
    _client.loop();

    // Tick counter for periodic connection check
    _mqttTick++;
    if (_mqttTick >= _loopsBeforeCheck) {
        _mqttTick = 0;
        if (!_client.connected()) {
            // Signal that higher-level state machine should reconnect
            return { Next::ReconnectNeeded, _loopIntervalMs };
        }
    }

    // Heartbeat: count "ticks" (calls to step) instead of measuring time
    _heartbeatTickCounter++;
    if (_heartbeatTickCounter >= _ticksPerHeartbeat) {
        _heartbeatTickCounter = 0;

        // Use current UTC time only for payload, not for scheduling
        time_t now = time(nullptr);
        uint32_t epochToPublish = (now > 0) ? static_cast<uint32_t>(now) : 0;
        if (epochToPublish != 0) {
            _lastHeartbeatEpoch = epochToPublish;
        }

        publishHeartbeat(epochToPublish);
    }

    return { Next::Stay, _loopIntervalMs };
}

bool HaRunningMonitor::publishHeartbeat(uint32_t epoch) {
    if (!_client.connected()) {
        LOG_INFO("MQTT not connected, skipping heartbeat publish");
        return false;
    }

    // Delegate topic and payload construction to MqttPublisher
    _client.loop();
    bool ok = _statusPublisher.publishOnlineHeartbeat(epoch);
    if (!ok) {
        LOG_ERROR("MQTT heartbeat publish FAILED");
    }
    return ok;
}
