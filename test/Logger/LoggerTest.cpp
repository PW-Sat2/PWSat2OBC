#include "Logger/Logger.h"
#include <string>
#include "gmock/gmock-matchers.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "system.h"

namespace
{
    struct LoggerEndpoint
    {
        MOCK_METHOD3(Log, void(LogLevel logLevel, const char* header, const char* message));
        MOCK_METHOD3(LogFormat, void(LogLevel logLevel, const char* header, const std::string& message));
    };
}

using testing::Eq;
using testing::Ge;
using testing::_;
using testing::StrEq;

static void LoggerProxy(void* context,
    enum LogLevel messageLevel,
    const char* messageHeader,
    const char* messageFormat,
    va_list messageArguments)
{
    UNREFERENCED_PARAMETER(messageArguments);
    const auto endpoint = static_cast<LoggerEndpoint*>(context);
    endpoint->Log(messageLevel, messageHeader, messageFormat);
}

static void LoggerProxyWithFormat(void* context,
    enum LogLevel messageLevel,
    const char* messageHeader,
    const char* messageFormat,
    va_list messageArguments)
{
    std::string message;
    auto result = vsnprintf(nullptr, 0, messageFormat, messageArguments) + 1;
    ASSERT_THAT(result, Ge(0));
    message.resize(result);

    result = vsnprintf(&message[0], message.size(), messageFormat, messageArguments);
    message.resize(result);
    const auto endpoint = static_cast<LoggerEndpoint*>(context);
    endpoint->LogFormat(messageLevel, messageHeader, message);
}

TEST(LoggerTest, TestAddSingleEndpoint)
{
    LoggerEndpoint endpoint;
    LogInit(LOG_LEVEL_INFO);
    const auto result = LogAddEndpoint(LoggerProxy, &endpoint, LOG_LEVEL_ERROR);
    ASSERT_THAT(result, Eq(true));
}

TEST(LoggerTest, TestAddMultipleEndpointsBeyondLimit)
{
    LoggerEndpoint endpoint;
    LogInit(LOG_LEVEL_INFO);
    LogAddEndpoint(LoggerProxy, &endpoint, LOG_LEVEL_ERROR);
    LogAddEndpoint(LoggerProxyWithFormat, &endpoint, LOG_LEVEL_ERROR);
    LogAddEndpoint(LoggerProxy, &endpoint, LOG_LEVEL_ERROR);
    const auto result = LogAddEndpoint(LoggerProxy, &endpoint, LOG_LEVEL_ERROR);
    ASSERT_THAT(result, Eq(false));
}

TEST(LoggerTest, TestLogEntryWith)
{
    LoggerEndpoint endpoint;
    EXPECT_CALL(endpoint, Log(LOG_LEVEL_FATAL, _, StrEq("Test Message")));
    LogInit(LOG_LEVEL_INFO);
    LogAddEndpoint(LoggerProxy, &endpoint, LOG_LEVEL_ERROR);
    LOG(LOG_LEVEL_FATAL, "Test Message");
}

TEST(LoggerTest, TestLogEntryWithFormat)
{
    LoggerEndpoint endpoint;
    EXPECT_CALL(endpoint, LogFormat(LOG_LEVEL_FATAL, _, StrEq("Test Message 1")));
    LogInit(LOG_LEVEL_INFO);
    LogAddEndpoint(LoggerProxyWithFormat, &endpoint, LOG_LEVEL_ERROR);
    LOGF(LOG_LEVEL_FATAL, "%s Message %d", "Test", 1);
}

TEST(LoggerTest, TestMultipleEndpoints)
{
    LoggerEndpoint endpoint;
    EXPECT_CALL(endpoint, LogFormat(LOG_LEVEL_FATAL, _, StrEq("Test Message 1")));
    EXPECT_CALL(endpoint, Log(LOG_LEVEL_FATAL, _, StrEq("%s Message %d")));
    LogInit(LOG_LEVEL_INFO);
    LogAddEndpoint(LoggerProxyWithFormat, &endpoint, LOG_LEVEL_ERROR);
    LogAddEndpoint(LoggerProxy, &endpoint, LOG_LEVEL_ERROR);
    LOGF(LOG_LEVEL_FATAL, "%s Message %d", "Test", 1);
}

TEST(LoggerTest, TestMessageOnDisabledLevel)
{
    LoggerEndpoint endpoint;
    EXPECT_CALL(endpoint, LogFormat(_, _, _)).Times(0);
    EXPECT_CALL(endpoint, Log(_, _, _)).Times(0);
    LogInit(LOG_LEVEL_INFO);
    LogAddEndpoint(LoggerProxyWithFormat, &endpoint, LOG_LEVEL_ERROR);
    LogAddEndpoint(LoggerProxy, &endpoint, LOG_LEVEL_ERROR);
    LOGF(LOG_LEVEL_DEBUG, "%s Message %d", "Test", 1);
}

TEST(LoggerTest, TestMessageOnDisabledLevelByEndpoint)
{
    LoggerEndpoint endpoint;
    EXPECT_CALL(endpoint, LogFormat(LOG_LEVEL_INFO, _, StrEq("Test Message 1")));
    EXPECT_CALL(endpoint, Log(_, _, _)).Times(0);
    LogInit(LOG_LEVEL_INFO);
    LogAddEndpoint(LoggerProxy, &endpoint, LOG_LEVEL_ERROR);
    LogAddEndpoint(LoggerProxyWithFormat, &endpoint, LOG_LEVEL_INFO);
    LOGF(LOG_LEVEL_INFO, "%s Message %d", "Test", 1);
}

TEST(LoggerTest, TestDuplicatedRegistration)
{
    LoggerEndpoint endpoint;
    EXPECT_CALL(endpoint, LogFormat(LOG_LEVEL_INFO, _, StrEq("Test Message 1"))).Times(2);
    EXPECT_CALL(endpoint, Log(_, _, _)).Times(0);
    LogInit(LOG_LEVEL_INFO);
    LogAddEndpoint(LoggerProxy, &endpoint, LOG_LEVEL_ERROR);
    LogAddEndpoint(LoggerProxyWithFormat, &endpoint, LOG_LEVEL_INFO);
    LogAddEndpoint(LoggerProxyWithFormat, &endpoint, LOG_LEVEL_DEBUG);
    LOGF(LOG_LEVEL_INFO, "%s Message %d", "Test", 1);
}

TEST(LoggerTest, TestDuplicatedRegistrationCheckLevels)
{
    LoggerEndpoint endpoint;
    EXPECT_CALL(endpoint, LogFormat(LOG_LEVEL_DEBUG, _, StrEq("Test Message 1"))).Times(1);
    EXPECT_CALL(endpoint, Log(_, _, _)).Times(0);
    LogInit(LOG_LEVEL_TRACE);
    LogAddEndpoint(LoggerProxy, &endpoint, LOG_LEVEL_ERROR);
    LogAddEndpoint(LoggerProxyWithFormat, &endpoint, LOG_LEVEL_INFO);
    LogAddEndpoint(LoggerProxyWithFormat, &endpoint, LOG_LEVEL_DEBUG);
    LOGF(LOG_LEVEL_DEBUG, "%s Message %d", "Test", 1);
}

TEST(LoggerTest, TestRemovingEndpoint)
{
    LoggerEndpoint endpoint;
    EXPECT_CALL(endpoint, LogFormat(LOG_LEVEL_FATAL, _, StrEq("Test Message 1"))).Times(2);
    EXPECT_CALL(endpoint, Log(_, _, _)).Times(0);
    LogInit(LOG_LEVEL_INFO);
    LogAddEndpoint(LoggerProxyWithFormat, &endpoint, LOG_LEVEL_INFO);
    LogAddEndpoint(LoggerProxyWithFormat, &endpoint, LOG_LEVEL_DEBUG);
    LogAddEndpoint(LoggerProxy, &endpoint, LOG_LEVEL_ERROR);
    LogRemoveEndpoint(LoggerProxy);
    LOGF(LOG_LEVEL_FATAL, "%s Message %d", "Test", 1);
}

TEST(LoggerTest, TestRemovingNonExistingEndpoint)
{
    LoggerEndpoint endpoint;
    EXPECT_CALL(endpoint, LogFormat(LOG_LEVEL_FATAL, _, StrEq("Test Message 1"))).Times(2);
    EXPECT_CALL(endpoint, Log(_, _, _)).Times(0);
    LogInit(LOG_LEVEL_INFO);
    LogAddEndpoint(LoggerProxyWithFormat, &endpoint, LOG_LEVEL_INFO);
    LogAddEndpoint(LoggerProxyWithFormat, &endpoint, LOG_LEVEL_DEBUG);
    LogRemoveEndpoint(LoggerProxy);
    LOGF(LOG_LEVEL_FATAL, "%s Message %d", "Test", 1);
}

TEST(LoggerTest, TestRemovingNullEndpint)
{
    LoggerEndpoint endpoint;
    EXPECT_CALL(endpoint, LogFormat(LOG_LEVEL_FATAL, _, StrEq("Test Message 1"))).Times(2);
    EXPECT_CALL(endpoint, Log(_, _, _)).Times(0);
    LogInit(LOG_LEVEL_INFO);
    LogAddEndpoint(LoggerProxyWithFormat, &endpoint, LOG_LEVEL_INFO);
    LogAddEndpoint(LoggerProxyWithFormat, &endpoint, LOG_LEVEL_DEBUG);
    LogRemoveEndpoint(NULL);
    LOGF(LOG_LEVEL_FATAL, "%s Message %d", "Test", 1);
}
