#include "Logger/Logger.h"
#include <string>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gmock/gmock-matchers.h"
#include "system.h"

namespace
{
    struct LoggerEndpoint
    {
        MOCK_METHOD3(Log, void(bool withinIsr, const char* header, const char* message));
        MOCK_METHOD3(LogFormat, void(bool withinIsr, const char* header, const std::string& message));
    };
}

using testing::Eq;
using testing::Ge;
using testing::_;
using testing::StrEq;

static void LoggerProxy(
    void* context, bool withinIsr, const char* messageHeader, const char* messageFormat, va_list messageArguments)
{
    UNREFERENCED_PARAMETER(messageArguments);
    const auto endpoint = static_cast<LoggerEndpoint*>(context);
    endpoint->Log(withinIsr, messageHeader, messageFormat);
}

static void LoggerProxyWithFormat(
    void* context, bool withinIsr, const char* messageHeader, const char* messageFormat, va_list messageArguments)
{
    std::string message;
    auto result = vsnprintf(nullptr, 0, messageFormat, messageArguments) + 1;
    ASSERT_THAT(result, Ge(0));
    message.resize(result);

    result = vsnprintf(&message[0], message.size(), messageFormat, messageArguments);
    message.resize(result);
    const auto endpoint = static_cast<LoggerEndpoint*>(context);
    endpoint->LogFormat(withinIsr, messageHeader, message);
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
    EXPECT_CALL(endpoint, Log(_, _, StrEq("Test Message")));
    LogInit(LOG_LEVEL_INFO);
    LogAddEndpoint(LoggerProxy, &endpoint, LOG_LEVEL_ERROR);
    LOG(LOG_LEVEL_FATAL, "Test Message");
}

TEST(LoggerTest, TestLogEntryWithFormat)
{
    LoggerEndpoint endpoint;
    EXPECT_CALL(endpoint, LogFormat(_, _, StrEq("Test Message 1")));
    LogInit(LOG_LEVEL_INFO);
    LogAddEndpoint(LoggerProxyWithFormat, &endpoint, LOG_LEVEL_ERROR);
    LOGF(LOG_LEVEL_FATAL, "%s Message %d", "Test", 1);
}

TEST(LoggerTest, TestMultipleEndpoints)
{
    LoggerEndpoint endpoint;
    EXPECT_CALL(endpoint, LogFormat(_, _, StrEq("Test Message 1")));
    EXPECT_CALL(endpoint, Log(_, _, StrEq("%s Message %d")));
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
    EXPECT_CALL(endpoint, LogFormat(_, _, StrEq("Test Message 1")));
    EXPECT_CALL(endpoint, Log(_, _, _)).Times(0);
    LogInit(LOG_LEVEL_INFO);
    LogAddEndpoint(LoggerProxy, &endpoint, LOG_LEVEL_ERROR);
    LogAddEndpoint(LoggerProxyWithFormat, &endpoint, LOG_LEVEL_INFO);
    LOGF(LOG_LEVEL_INFO, "%s Message %d", "Test", 1);
}

TEST(LoggerTest, TestDuplicatedRegistration)
{
    LoggerEndpoint endpoint;
    EXPECT_CALL(endpoint, LogFormat(_, _, StrEq("Test Message 1"))).Times(2);
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
    EXPECT_CALL(endpoint, LogFormat(_, _, StrEq("Test Message 1"))).Times(1);
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
    EXPECT_CALL(endpoint, LogFormat(_, _, StrEq("Test Message 1"))).Times(2);
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
    EXPECT_CALL(endpoint, LogFormat(_, _, StrEq("Test Message 1"))).Times(2);
    EXPECT_CALL(endpoint, Log(_, _, _)).Times(0);
    LogInit(LOG_LEVEL_INFO);
    LogAddEndpoint(LoggerProxyWithFormat, &endpoint, LOG_LEVEL_INFO);
    LogAddEndpoint(LoggerProxyWithFormat, &endpoint, LOG_LEVEL_DEBUG);
    LogRemoveEndpoint(LoggerProxy);
    LOGF(LOG_LEVEL_FATAL, "%s Message %d", "Test", 1);
}

TEST(LoggerTest, TestRemovingNullEndpoint)
{
    LoggerEndpoint endpoint;
    EXPECT_CALL(endpoint, LogFormat(_, _, StrEq("Test Message 1"))).Times(2);
    EXPECT_CALL(endpoint, Log(_, _, _)).Times(0);
    LogInit(LOG_LEVEL_INFO);
    LogAddEndpoint(LoggerProxyWithFormat, &endpoint, LOG_LEVEL_INFO);
    LogAddEndpoint(LoggerProxyWithFormat, &endpoint, LOG_LEVEL_DEBUG);
    LogRemoveEndpoint(NULL);
    LOGF(LOG_LEVEL_FATAL, "%s Message %d", "Test", 1);
}

TEST(LoggerTest, TestIsrFlagStandardMacroNoFormat)
{
    LoggerEndpoint endpoint;
    EXPECT_CALL(endpoint, Log(false, _, _)).Times(1);
    LogInit(LOG_LEVEL_INFO);
    LogAddEndpoint(LoggerProxy, &endpoint, LOG_LEVEL_INFO);
    LOG(LOG_LEVEL_FATAL, "Message");
}

TEST(LoggerTest, TestIsrFlagStandardMacroWithFormat)
{
    LoggerEndpoint endpoint;
    EXPECT_CALL(endpoint, Log(0, _, _)).Times(1);
    LogInit(LOG_LEVEL_INFO);
    LogAddEndpoint(LoggerProxy, &endpoint, LOG_LEVEL_INFO);
    LOGF(LOG_LEVEL_FATAL, "%s Message %d", "My", 1);
}

TEST(LoggerTest, TestIsrFlagIsrMacroNoFormat)
{
    LoggerEndpoint endpoint;
    EXPECT_CALL(endpoint, Log(true, _, _)).Times(1);
    LogInit(LOG_LEVEL_INFO);
    LogAddEndpoint(LoggerProxy, &endpoint, LOG_LEVEL_INFO);
    LOG_ISR(LOG_LEVEL_FATAL, "Message");
}

TEST(LoggerTest, TestIsrFlagIsrMacroWithFormat)
{
    LoggerEndpoint endpoint;
    EXPECT_CALL(endpoint, Log(true, _, _)).Times(1);
    LogInit(LOG_LEVEL_INFO);
    LogAddEndpoint(LoggerProxy, &endpoint, LOG_LEVEL_INFO);
    LOGF_ISR(LOG_LEVEL_FATAL, "%s Message %d", "My", 1);
}

TEST(LoggerTest, TestIsrFlagImmediateMacroNoFormat)
{
    LoggerEndpoint endpoint;
    EXPECT_CALL(endpoint, Log(true, _, _)).Times(1);
    EXPECT_CALL(endpoint, Log(false, _, _)).Times(1);
    LogInit(LOG_LEVEL_INFO);
    LogAddEndpoint(LoggerProxy, &endpoint, LOG_LEVEL_INFO);
    LOGI(true, LOG_LEVEL_FATAL, "Message");
    LOGI(false, LOG_LEVEL_FATAL, "Message");
}

TEST(LoggerTest, TestIsrFlagImmediateMacroWithFormat)
{
    LoggerEndpoint endpoint;
    EXPECT_CALL(endpoint, Log(true, _, _)).Times(1);
    EXPECT_CALL(endpoint, Log(false, _, _)).Times(1);
    LogInit(LOG_LEVEL_INFO);
    LogAddEndpoint(LoggerProxy, &endpoint, LOG_LEVEL_INFO);
    LOGFI(false, LOG_LEVEL_FATAL, "%s Message %d", "My", 1);
    LOGFI(true, LOG_LEVEL_FATAL, "%s Message %d", "My", 1);
}
