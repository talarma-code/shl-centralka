#pragma once

#include <Arduino.h>
#include "Logger.h"

inline uint32_t LogNowSeconds() {
    return static_cast<uint32_t>(time(nullptr));
}

// --- Debug ---
inline void LogDebug(const char* msg) {
    Logger::instance().log(LogLevel::debug, LogNowSeconds(), msg);
}

template<typename... Args>
inline void LogDebugf(const char* fmt, Args... args) {
    Logger::instance().logf(LogLevel::debug, LogNowSeconds(), fmt, args...);
}

// --- Detail debug ---
inline void LogDetailDebug(const char* msg) {
    Logger::instance().log(LogLevel::detailDebug, LogNowSeconds(), msg);
}
template<typename... Args>
inline void LogDetailDebugf(const char* fmt, Args... args) {
    Logger::instance().logf(LogLevel::detailDebug, LogNowSeconds(), fmt, args...);
}

// --- Info ---
inline void LogInfo(const char* msg) {
    Logger::instance().log(LogLevel::info, LogNowSeconds(), msg);
}

template<typename... Args>
inline void LogInfof(const char* fmt, Args... args) {
    Logger::instance().logf(LogLevel::info, LogNowSeconds(), fmt, args...);
}

// --- Error ---
inline void LogError(const char* msg) {
    Logger::instance().log(LogLevel::error, LogNowSeconds(), msg);
}

template<typename... Args>
inline void LogErrorf(const char* fmt, Args... args) {
    Logger::instance().logf(LogLevel::error, LogNowSeconds(), fmt, args...);
}

// --- Critical ---
inline void LogCritical(const char* msg) {
    Logger::instance().log(LogLevel::critical, LogNowSeconds(), msg);
}

template<typename... Args>
inline void LogCriticalf(const char* fmt, Args... args) {
    Logger::instance().logf(LogLevel::critical, LogNowSeconds(), fmt, args...);
}


#define LOG_DEBUG(fmt, ...)    Logger::instance().logf(LogLevel::debug,    LogNowSeconds(), fmt, ##__VA_ARGS__)
#define LOG_DETAIL(fmt, ...)   Logger::instance().logf(LogLevel::detailDebug, LogNowSeconds(), fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)     Logger::instance().logf(LogLevel::info,     LogNowSeconds(), fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...)    Logger::instance().logf(LogLevel::error,    LogNowSeconds(), fmt, ##__VA_ARGS__)
#define LOG_CRITICAL(fmt, ...) Logger::instance().logf(LogLevel::critical, LogNowSeconds(), fmt, ##__VA_ARGS__)
