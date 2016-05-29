#include "Logger.h"
#include <assert.h> // static_assert
#include <stdint.h>
#include <stdio.h>
#include <string.h> //memset
#include "system.h"

#define MAX_ENDPOINTS 3

static_assert(MAX_ENDPOINTS < UINT8_MAX, "Fix type of logger's endpoint counter: 'Logger::endpointCount'. ");

typedef struct LoggerEndpointTag
{
    void* context;
    LoggerProcedure endpoint;
    enum LogLevel endpointLogLevel;
} LoggerEndpoint;

typedef struct LoggerTag
{
    uint8_t endpointCount;
    enum LogLevel globalLevel;
    LoggerEndpoint endpoints[MAX_ENDPOINTS];
} Logger;

static Logger logger = {0};

static const char* const levelMap[] = {
    "[Always] ", "[Fatal] ", "[Error] ", "[Warning] ", "[Info] ", "[Debug] ", "[Trace] ",
};

static_assert(LOG_LEVEL_ALWAYS == 0, "Fix level conversion map for level: Always");
static_assert(LOG_LEVEL_FATAL == 1, "Fix level conversion map for level: Fatal");
static_assert(LOG_LEVEL_ERROR == 2, "Fix level conversion map for level: Error");
static_assert(LOG_LEVEL_WARNING == 3, "Fix level conversion map for level: Warning");
static_assert(LOG_LEVEL_INFO == 4, "Fix level conversion map for level: Info");
static_assert(LOG_LEVEL_DEBUG == 5, "Fix level conversion map for level: Debug");
static_assert(LOG_LEVEL_TRACE == 6, "Fix level conversion map for level: Trace");

static const char* LogConvertLevelToString(enum LogLevel level)
{
    if (level >= COUNT_OF(levelMap))
    {
        return "[Unknown] ";
    }
    else
    {
        return levelMap[level];
    }
}

void LogInit(enum LogLevel globalLogLevel)
{
    logger.globalLevel = globalLogLevel;
    logger.endpointCount = 0;
    memset(logger.endpoints, 0, sizeof(logger.endpoints));
}

bool LogAddEndpoint(LoggerProcedure endpoint, void* context, enum LogLevel endpointLogLevel)
{
    if (logger.endpointCount >= MAX_ENDPOINTS)
    {
        return false;
    }

    LoggerEndpoint endpointDescription;
    endpointDescription.endpoint = endpoint;
    endpointDescription.context = context;
    endpointDescription.endpointLogLevel = endpointLogLevel;

    logger.endpoints[logger.endpointCount++] = endpointDescription;
    return true;
}

void LogRemoveEndpoint(LoggerProcedure endpoint)
{
    for (uint8_t cx = 0; cx < logger.endpointCount; ++cx)
    {
        if (logger.endpoints[cx].endpoint == endpoint)
        {
            memmove(logger.endpoints + cx,
                logger.endpoints + cx + 1,
                sizeof(*logger.endpoints) * (logger.endpointCount - (cx + 1)));
            --logger.endpointCount;
            break;
        }
    }
}

static inline bool CanLogAtLevel(const enum LogLevel requestedLogLEvel, const enum LogLevel currentLogLevel)
{
    return requestedLogLEvel <= currentLogLevel;
}

void LogMessage(enum LogLevel messageLevel, const char* message, ...)
{
    if (!CanLogAtLevel(messageLevel, logger.globalLevel))
    {
        return;
    }

    const char* header = LogConvertLevelToString(messageLevel);

    va_list arguments;
    va_start(arguments, message);

    for (uint8_t cx = 0; cx < logger.endpointCount; ++cx)
    {
        const LoggerEndpoint* endpoint = &logger.endpoints[cx];
        if (CanLogAtLevel(messageLevel, endpoint->endpointLogLevel))
        {
            endpoint->endpoint(endpoint->context, header, message, arguments);
        }
    }

    va_end(arguments);
}
