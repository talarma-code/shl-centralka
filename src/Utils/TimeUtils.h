#pragma once

#include <stdint.h>
#include "GlobalTypes.h"

// Format epoch seconds to ISO 8601 string: YYYY-MM-DDTHH:MM:SS
StaticString32 formatIsoTimestamp(uint32_t epoch);

// Parse ISO 8601 string (YYYY-MM-DDTHH:MM[:SS]) to epoch seconds
bool parseIsoTimestamp(const char* str, uint32_t &outEpoch);
