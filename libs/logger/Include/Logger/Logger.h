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
    LOG_LEVEL_ALWAYS = 0,
    LOG_LEVEL_FATAL = 1,
    LOG_LEVEL_ERROR = 2,
    LOG_LEVEL_WARNING = 3,
    LOG_LEVEL_INFO = 4,
    LOG_LEVEL_DEBUG = 5,
    LOG_LEVEL_TRACE = 6
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
