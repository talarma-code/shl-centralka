#pragma once
#include "StaticString.h"

using StaticString192 = StaticString<192>;
using StaticString32 = StaticString<32>;
using StaticString96 = StaticString<96>;

static const uint16_t INTERVAL_3_MINUTES_MS = 180000; 
static const uint16_t INTERVAL_30_SECONDS_MS = 30000;
static const uint16_t INTERVAL_5_SECONDS_MS = 5000;
static const uint16_t INTERVAL_2_SECONDS_MS = 2000;
static const uint16_t INTERVAL_100_MS = 100;
static const uint16_t DO_NOT_RUN_TIMER = 0xFFFF;