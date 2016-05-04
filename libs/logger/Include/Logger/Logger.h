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

/**
 * @brief Function pointer type that defines entry point for logger's endpoint entry point.
 *
 * @param[in] context Pointer to endpoint's specific context. This value is provided during endpoint registration.
 * @param[in] messageHeader Pointer to string that contains log entry header.
 * @param[in] messageFormat Pointer to logged message format string.
 * @param[in] messageArguments Logged message arguments.
 */
typedef void (*LoggerProcedure)(
    void* context, const char* messageHeader, const char* messageFormat, va_list messageArguments);

/**
 * @brief Macro used for logging non parameterized entries.
 */
#define LOG(level, message) LogMessage(level, __FILE__, __LINE__, message)

/**
 * @brief Macro used for logging parameterized entries.
 */
#define LOGF(level, message, ...) LogMessage(level, __FILE__, __LINE__, message, __VA_ARGS__)

/**
 * @brief Reinitializes the logger.
 *
 * @param globalLogLevel Requested global logger logging level.
 *
 * The logging level provided to this function is applied uniformly to
 * all logger endpoints. Log entries below this logging level (with higher level value)
 * will not be logged at all. Log entries on or above this level will be logged to all
 * endpoints whose logging level allows it.
 *
 * @remark Initializing already initialized logger sets it back to default state.
 */
void LogInit(enum LogLevel globalLogLevel);

/**
 * @brief Adds logger endpoint.
 *
 * @param[in] endpoint Pointer to function that should be used for logging.
 * @param[in] context Pointer to the endpoint specific structure that will be passed to the
 *  endpoint logging procedure as its context parameter. Logger does not use this value.
 *  This parameter can be NULL.
 * @param[in] endpointLogLevel Endpoint specific logging level. Use this value to filter out
 * log entries on the endpoint level.
 *
 * @return Operation status.
 * @retval true On success.
 * @retval false when another endpoint could not be added to logger.
 *
 * The endpointLogLevel parameter cannot be used to include log entries that are filtered out
 * by the global log level parameter. It can only be used to futher limit the number of log entries
 * that are send to the specific endpoint.
 *
 * Registering the same endpoint multiple times is allowed. In such case the specific endpoint will
 * be asked multiple times to log the same log entry. All registrations are independent from
 * each other and can use different contexts and logging levels.
 */
bool LogAddEndpoint(LoggerProcedure endpoint, void* context, enum LogLevel endpointLogLevel);

/**
 * @brief Removes requested logging endpoint.
 *
 * @param[in] endpoint Pointer to endpoint function that was previously registered.
 *
 * @remarks Specific endpoint has to be removed the same number of times it was added to the logger
 * to make it completely inactive.
 *
 * When requested endpoint has been registered multiple times, then the remove request will remove
 * only the first endpoint that matches the criteria.
 *
 * Removing non existing endpoint has no effect.
 */
void LogRemoveEndpoint(LoggerProcedure endpoint);

/**
 * @brief Logs a message.
 *
 * @param[in] messageLevel Requested message logging level.
 * @param[in] file Current source file name.
 * @param[in] line Current line.
 * @param[in] message Logged message.
 */
void LogMessage(enum LogLevel messageLevel, const char* file, uint32_t line, const char* message, ...);

#ifdef __cplusplus
}
#endif

#endif
