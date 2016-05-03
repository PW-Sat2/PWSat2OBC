#ifndef SRC_LOGGER_H
#define SRC_LOGGER_H

#pragma once

#include <stdarg.h> // va_list
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum LogLevel
{
    // dummy log level used for verification, do not use it, keep it first on the list
    LOG_LEVEL_MIN = 0,

    LOG_LEVEL_ALWAYS = LOG_LEVEL_MIN,
    LOG_LEVEL_FATAL,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_TRACE,

    // dummy log level used for verification, do not use it, keep it last on the list
    LOG_LEVEL_MAX = LOG_LEVEL_TRACE
};

typedef void (*LoggerProcedure)(void* context,
    enum LogLevel messageLevel,
    const char* messageHeader,
    const char* messageFormat,
    va_list messageArguments);

#define LOG(level, message) LogMessage(level, __FILE__, __LINE__, message)

#define LOGF(level, message, ...) LogMessage(level, __FILE__, __LINE__, message, __VA_ARGS__)

void LogInit(enum LogLevel globalLogLevel);

bool LogAddEndpoint(LoggerProcedure endpoint, void* context, enum LogLevel endpointLogLevel);

void LogRemoveEndpoint(LoggerProcedure endpoint);

void LogMessage(enum LogLevel messageLevel, const char* file, uint32_t line, const char* message, ...);

#ifdef __cplusplus
}
#endif

#endif
