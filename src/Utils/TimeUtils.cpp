#include "Utils/TimeUtils.h"
#include <time.h>
#include <stdio.h>
#include <string.h>

StaticString32 formatIsoTimestamp(uint32_t epoch) {
    time_t t = (time_t)epoch;
    struct tm tm_val;
#if defined(__GNUC__)
    gmtime_r(&t, &tm_val);
#else
    struct tm *tmp = gmtime(&t);
    if (tmp) tm_val = *tmp; else memset(&tm_val, 0, sizeof(tm_val));
#endif
    char buf[32];
    // ISO 8601 in UTC with explicit "Z" suffix so Home Assistant
    // timestamp device_class parses it correctly, e.g. 2026-01-15T15:11:37Z
    snprintf(buf, sizeof(buf), "%04d-%02d-%02dT%02d:%02d:%02dZ",
        tm_val.tm_year + 1900, tm_val.tm_mon + 1, tm_val.tm_mday,
        tm_val.tm_hour, tm_val.tm_min, tm_val.tm_sec);

    StaticString32 s;
    s.assign(buf);
    return s;
}

uint32_t convertLocalEpochToUtc(uint32_t localEpoch, int8_t offsetHours) {
    // Device clock runs in local time (e.g. UTC+1). To get true UTC epoch
    // subtract the offset (in seconds) from the local epoch value.
    if (offsetHours == 0) {
        return localEpoch;
    }

    int32_t offsetSec = (int32_t)offsetHours * 3600;

    if (offsetSec > 0) {
        // Local time = UTC + offset  ->  UTC = local - offset
        if (localEpoch < (uint32_t)offsetSec) {
            return 0;
        }
        return localEpoch - (uint32_t)offsetSec;
    } else {
        // Negative offset (e.g. UTC-3)
        uint32_t absOffset = (uint32_t)(-offsetSec);
        return localEpoch + absOffset;
    }
}

bool parseIsoTimestamp(const char* str, uint32_t &outEpoch) {
    if (!str) return false;
    int year=0, mon=0, day=0, hour=0, min=0, sec=0;
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

    time_t tt = mktime(&tm_val);
    if (tt < 0) return false;
    outEpoch = (uint32_t)tt;
    return true;
}
