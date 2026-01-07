#include "SimpleJsonParser.h"
#include <time.h>
#include <stdio.h>
#include <string.h>

// Helper: format epoch -> ISO timestamp
StaticString192 SimpleJsonParser::formatIsoTimestamp(uint32_t epoch) {
    time_t t = (time_t)epoch;
    struct tm tm_val;
#if defined(__GNUC__)
    gmtime_r(&t, &tm_val);
#else
    struct tm *tmp = gmtime(&t);
    if (tmp) tm_val = *tmp; else memset(&tm_val, 0, sizeof(tm_val));
#endif
    char buf[32];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02dT%02d:%02d:%02d",
        tm_val.tm_year + 1900, tm_val.tm_mon + 1, tm_val.tm_mday,
        tm_val.tm_hour, tm_val.tm_min, tm_val.tm_sec);

    StaticString192 s;
    s.assign(buf);
    return s;
}

// Helper: parse ISO timestamp (supports YYYY-MM-DDTHH:MM and optional :SS)
bool SimpleJsonParser::parseIsoTimestamp(const char* str, uint32_t &outEpoch) {
    if (!str) return false;
    int year=0, mon=0, day=0, hour=0, min=0, sec=0;
    // Try with seconds
    int matched = sscanf(str, "%4d-%2d-%2dT%2d:%2d:%2d", &year, &mon, &day, &hour, &min, &sec);
    if (matched < 5) return false;

    struct tm tm_val;
    memset(&tm_val, 0, sizeof(tm_val));
    tm_val.tm_year = year - 1900;
    tm_val.tm_mon = mon - 1;
    tm_val.tm_mday = day;
    tm_val.tm_hour = hour;
    tm_val.tm_min = min;
    tm_val.tm_sec = sec;

    time_t tt = mktime(&tm_val); // local time -> epoch
    if (tt < 0) return false;
    outEpoch = (uint32_t)tt;
    return true;
}

// -------------------- Measurement --------------------
StaticString192 SimpleJsonParser::serializeMeasurement(const MeasurementDataPacket& m) {
    StaticString192 ts = formatIsoTimestamp(m.timestamp);
    char buf[128];
    int n = snprintf(buf, sizeof(buf), "{\"timestamp\":\"%s\",\"L1Power\":%u,\"L2Power\":%u,\"HeaterPower\":%u,\"HomeTotalPower\":%u,\"L1Voltage_x10\":%u,\"L2Voltage_x10\":%u}",
        ts.c_str(), (unsigned)m.L1Power, (unsigned)m.L2Power, (unsigned)m.HeaterPower, (unsigned)m.HomeTotalPower,
        (unsigned)m.L1Voltage_x10, (unsigned)m.L2Voltage_x10);
    StaticString192 s;
    if (n > 0) s.assign(buf); else s.clear();
    return s;
}

bool SimpleJsonParser::deserializeMeasurement(const StaticString192& json, MeasurementDataPacket& out) {
    const char* j = json.c_str();
    auto extractUInt = [&](const char* key)->uint32_t {
        const char* p = strstr(j, key);
        if (!p) return 0;
        p = strchr(p, ':'); if (!p) return 0; p++;
        while (*p && (*p == ' ' || *p == '"')) p++;
        char* endptr;
        uint32_t val = (uint32_t) strtoul(p, &endptr, 10);
        return val;
    };

    // timestamp is a string (ISO) -> parse
    const char* tpos = strstr(j, "\"timestamp\"");
    if (!tpos) return false;
    tpos = strchr(tpos, ':'); if (!tpos) return false; tpos++;
    while (*tpos && (*tpos == ' ')) tpos++;
    if (*tpos != '"') return false; tpos++;
    const char* tend = strchr(tpos, '"'); if (!tend) return false;
    char tsStr[32];
    size_t tlen = tend - tpos;
    if (tlen >= sizeof(tsStr)) tlen = sizeof(tsStr) - 1;
    memcpy(tsStr, tpos, tlen); tsStr[tlen] = '\0';

    uint32_t epoch = 0;
    if (!parseIsoTimestamp(tsStr, epoch)) return false;
    out.timestamp = epoch;

    out.L1Power = extractUInt("\"L1Power\"");
    out.L2Power = extractUInt("\"L2Power\"");
    out.HeaterPower = extractUInt("\"HeaterPower\"");
    out.HomeTotalPower = extractUInt("\"HomeTotalPower\"");
    out.L1Voltage_x10 = (uint16_t)extractUInt("\"L1Voltage_x10\"");
    out.L2Voltage_x10 = (uint16_t)extractUInt("\"L2Voltage_x10\"");

    return out.timestamp != 0;
}

// -------------------- Event --------------------
static const char* reasonToString(esp_reset_reason_t reason) {
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
        default:                    return "Invalid Reason";
    }
}

static esp_reset_reason_t stringToReason(const char* s) {
    if (!s) return ESP_RST_UNKNOWN;
    if (strcmp(s, "Power On Reset") == 0) return ESP_RST_POWERON;
    if (strcmp(s, "External Reset") == 0) return ESP_RST_EXT;
    if (strcmp(s, "Software Reset") == 0) return ESP_RST_SW;
    if (strcmp(s, "Panic Reset") == 0) return ESP_RST_PANIC;
    if (strcmp(s, "Interrupt Watchdog Reset") == 0) return ESP_RST_INT_WDT;
    if (strcmp(s, "Task Watchdog Reset") == 0) return ESP_RST_TASK_WDT;
    if (strcmp(s, "Other Watchdog Reset") == 0) return ESP_RST_WDT;
    if (strcmp(s, "Deep Sleep Reset") == 0) return ESP_RST_DEEPSLEEP;
    if (strcmp(s, "Brownout Reset") == 0) return ESP_RST_BROWNOUT;
    if (strcmp(s, "SDIO Reset") == 0) return ESP_RST_SDIO;
    return ESP_RST_UNKNOWN;
}

StaticString192 SimpleJsonParser::serializeEvent(const SystemEventPacket& e) {
    StaticString192 ts = formatIsoTimestamp(e.timestamp);
    const char* r = reasonToString(e.reason);
    char buf[128];
    int n = snprintf(buf, sizeof(buf), "{\"timestamp\":\"%s\",\"reason\":\"%s\"}", ts.c_str(), r);
    StaticString192 s; if (n > 0) s.assign(buf); else s.clear();
    return s;
}

bool SimpleJsonParser::deserializeEvent(const StaticString192& json, SystemEventPacket& out) {
    const char* j = json.c_str();
    // timestamp
    const char* tpos = strstr(j, "\"timestamp\"");
    if (!tpos) return false;
    tpos = strchr(tpos, ':'); if (!tpos) return false; tpos++;
    while (*tpos && (*tpos == ' ')) tpos++;
    if (*tpos != '"') return false; tpos++;
    const char* tend = strchr(tpos, '"'); if (!tend) return false;
    char tsStr[32]; size_t tlen = tend - tpos; if (tlen >= sizeof(tsStr)) tlen = sizeof(tsStr)-1;
    memcpy(tsStr, tpos, tlen); tsStr[tlen] = '\0';
    uint32_t epoch = 0;
    if (!parseIsoTimestamp(tsStr, epoch)) return false;
    out.timestamp = epoch;
    // reason
    const char* rpos = strstr(j, "\"reason\""); if (!rpos) return false;
    rpos = strchr(rpos, ':'); if (!rpos) return false; rpos++;
    while (*rpos && (*rpos == ' ')) rpos++;
    if (*rpos != '"') return false; rpos++;
    const char* rend = strchr(rpos, '"'); if (!rend) return false;
    char rStr[64]; size_t rlen = rend - rpos; if (rlen >= sizeof(rStr)) rlen = sizeof(rStr)-1;
    memcpy(rStr, rpos, rlen); rStr[rlen] = '\0';
    out.reason = stringToReason(rStr);
    return true;
}
