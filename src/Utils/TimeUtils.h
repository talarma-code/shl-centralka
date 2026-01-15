#pragma once

#include <stdint.h>
#include "GlobalTypes.h"

// Format epoch seconds (UTC) to ISO 8601 string: YYYY-MM-DDTHH:MM:SSZ
StaticString32 formatIsoTimestamp(uint32_t epoch);

// Parse ISO 8601 string (YYYY-MM-DDTHH:MM[:SS]) to epoch seconds
bool parseIsoTimestamp(const char* str, uint32_t &outEpoch);

// Convert local epoch seconds to UTC by applying fixed offset in hours
// Example: for device clock in CET (UTC+1) pass offsetHours = 1
uint32_t convertLocalEpochToUtc(uint32_t localEpoch, int8_t offsetHours);
