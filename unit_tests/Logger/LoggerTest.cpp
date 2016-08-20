#include <string>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gmock/gmock-matchers.h"
#include "logger/logger.h"
#include "system.h"

using testing::Eq;
using testing::Ge;
using testing::_;
using testing::StrEq;

namespace
{
    struct LoggerEndpoint
    {
        MOCK_METHOD3(Log, void(bool withinIsr, const char* header, const char* message));
        MOCK_METHOD3(LogFormat, void(bool withinIsr, const char* header, const std::string& message));
    };
}

struct LogReset
{
    ~LogReset()
    {
        LogInit(LOG_LEVEL_ALWAYS);
    }
};

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

class LoggerTest : public testing::Test
{
  protected:
    LogReset guard;
    LoggerEndpoint endpoint;
};

TEST_F(LoggerTest, TestAddSingleEndpoint)
{
    LogInit(LOG_LEVEL_INFO);
    const auto result = LogAddEndpoint(LoggerProxy, &endpoint, LOG_LEVEL_ERROR);
    ASSERT_THAT(result, Eq(true));
}

TEST_F(LoggerTest, TestAddMultipleEndpointsBeyondLimit)
{
    LogInit(LOG_LEVEL_INFO);
    LogAddEndpoint(LoggerProxy, &endpoint, LOG_LEVEL_ERROR);
    LogAddEndpoint(LoggerProxyWithFormat, &endpoint, LOG_LEVEL_ERROR);
    LogAddEndpoint(LoggerProxy, &endpoint, LOG_LEVEL_ERROR);
    const auto result = LogAddEndpoint(LoggerProxy, &endpoint, LOG_LEVEL_ERROR);
    ASSERT_THAT(result, Eq(false));
}

TEST_F(LoggerTest, TestLogEntryWith)
{
    EXPECT_CALL(endpoint, Log(_, _, StrEq("Test Message")));
    LogInit(LOG_LEVEL_INFO);
    LogAddEndpoint(LoggerProxy, &endpoint, LOG_LEVEL_ERROR);
    LOG(LOG_LEVEL_FATAL, "Test Message");
}

TEST_F(LoggerTest, TestLogEntryWithFormat)
{
    EXPECT_CALL(endpoint, LogFormat(_, _, StrEq("Test Message 1")));
    LogInit(LOG_LEVEL_INFO);
    LogAddEndpoint(LoggerProxyWithFormat, &endpoint, LOG_LEVEL_ERROR);
    LOGF(LOG_LEVEL_FATAL, "%s Message %d", "Test", 1);
}

TEST_F(LoggerTest, TestMultipleEndpoints)
{
    EXPECT_CALL(endpoint, LogFormat(_, _, StrEq("Test Message 1")));
    EXPECT_CALL(endpoint, Log(_, _, StrEq("%s Message %d")));
    LogInit(LOG_LEVEL_INFO);
    LogAddEndpoint(LoggerProxyWithFormat, &endpoint, LOG_LEVEL_ERROR);
    LogAddEndpoint(LoggerProxy, &endpoint, LOG_LEVEL_ERROR);
    LOGF(LOG_LEVEL_FATAL, "%s Message %d", "Test", 1);
}

TEST_F(LoggerTest, TestMessageOnDisabledLevel)
{
    EXPECT_CALL(endpoint, LogFormat(_, _, _)).Times(0);
    EXPECT_CALL(endpoint, Log(_, _, _)).Times(0);
    LogInit(LOG_LEVEL_INFO);
    LogAddEndpoint(LoggerProxyWithFormat, &endpoint, LOG_LEVEL_ERROR);
    LogAddEndpoint(LoggerProxy, &endpoint, LOG_LEVEL_ERROR);
    LOGF(LOG_LEVEL_DEBUG, "%s Message %d", "Test", 1);
}

TEST_F(LoggerTest, TestMessageOnDisabledLevelByEndpoint)
{
    EXPECT_CALL(endpoint, LogFormat(_, _, StrEq("Test Message 1")));
    EXPECT_CALL(endpoint, Log(_, _, _)).Times(0);
    LogInit(LOG_LEVEL_INFO);
    LogAddEndpoint(LoggerProxy, &endpoint, LOG_LEVEL_ERROR);
    LogAddEndpoint(LoggerProxyWithFormat, &endpoint, LOG_LEVEL_INFO);
    LOGF(LOG_LEVEL_INFO, "%s Message %d", "Test", 1);
}

TEST_F(LoggerTest, TestDuplicatedRegistration)
{
    EXPECT_CALL(endpoint, LogFormat(_, _, StrEq("Test Message 1"))).Times(2);
    EXPECT_CALL(endpoint, Log(_, _, _)).Times(0);
    LogInit(LOG_LEVEL_INFO);
    LogAddEndpoint(LoggerProxy, &endpoint, LOG_LEVEL_ERROR);
    LogAddEndpoint(LoggerProxyWithFormat, &endpoint, LOG_LEVEL_INFO);
    LogAddEndpoint(LoggerProxyWithFormat, &endpoint, LOG_LEVEL_DEBUG);
    LOGF(LOG_LEVEL_INFO, "%s Message %d", "Test", 1);
}

TEST_F(LoggerTest, TestDuplicatedRegistrationCheckLevels)
{
    EXPECT_CALL(endpoint, LogFormat(_, _, StrEq("Test Message 1"))).Times(1);
    EXPECT_CALL(endpoint, Log(_, _, _)).Times(0);
    LogInit(LOG_LEVEL_TRACE);
    LogAddEndpoint(LoggerProxy, &endpoint, LOG_LEVEL_ERROR);
    LogAddEndpoint(LoggerProxyWithFormat, &endpoint, LOG_LEVEL_INFO);
    LogAddEndpoint(LoggerProxyWithFormat, &endpoint, LOG_LEVEL_DEBUG);
    LOGF(LOG_LEVEL_DEBUG, "%s Message %d", "Test", 1);
}

TEST_F(LoggerTest, TestRemovingEndpoint)
{
    EXPECT_CALL(endpoint, LogFormat(_, _, StrEq("Test Message 1"))).Times(2);
    EXPECT_CALL(endpoint, Log(_, _, _)).Times(0);
    LogInit(LOG_LEVEL_INFO);
    LogAddEndpoint(LoggerProxyWithFormat, &endpoint, LOG_LEVEL_INFO);
    LogAddEndpoint(LoggerProxyWithFormat, &endpoint, LOG_LEVEL_DEBUG);
    LogAddEndpoint(LoggerProxy, &endpoint, LOG_LEVEL_ERROR);
    LogRemoveEndpoint(LoggerProxy);
    LOGF(LOG_LEVEL_FATAL, "%s Message %d", "Test", 1);
}

TEST_F(LoggerTest, TestRemovingNonExistingEndpoint)
{
    EXPECT_CALL(endpoint, LogFormat(_, _, StrEq("Test Message 1"))).Times(2);
    EXPECT_CALL(endpoint, Log(_, _, _)).Times(0);
    LogInit(LOG_LEVEL_INFO);
    LogAddEndpoint(LoggerProxyWithFormat, &endpoint, LOG_LEVEL_INFO);
    LogAddEndpoint(LoggerProxyWithFormat, &endpoint, LOG_LEVEL_DEBUG);
    LogRemoveEndpoint(LoggerProxy);
    LOGF(LOG_LEVEL_FATAL, "%s Message %d", "Test", 1);
}

TEST_F(LoggerTest, TestRemovingNullEndpoint)
{
    EXPECT_CALL(endpoint, LogFormat(_, _, StrEq("Test Message 1"))).Times(2);
    EXPECT_CALL(endpoint, Log(_, _, _)).Times(0);
    LogInit(LOG_LEVEL_INFO);
    LogAddEndpoint(LoggerProxyWithFormat, &endpoint, LOG_LEVEL_INFO);
    LogAddEndpoint(LoggerProxyWithFormat, &endpoint, LOG_LEVEL_DEBUG);
    LogRemoveEndpoint(NULL);
    LOGF(LOG_LEVEL_FATAL, "%s Message %d", "Test", 1);
}

TEST_F(LoggerTest, TestIsrFlagStandardMacroNoFormat)
{
    EXPECT_CALL(endpoint, Log(false, _, _)).Times(1);
    LogInit(LOG_LEVEL_INFO);
    LogAddEndpoint(LoggerProxy, &endpoint, LOG_LEVEL_INFO);
    LOG(LOG_LEVEL_FATAL, "Message");
}

TEST_F(LoggerTest, TestIsrFlagStandardMacroWithFormat)
{
    EXPECT_CALL(endpoint, Log(0, _, _)).Times(1);
    LogInit(LOG_LEVEL_INFO);
    LogAddEndpoint(LoggerProxy, &endpoint, LOG_LEVEL_INFO);
    LOGF(LOG_LEVEL_FATAL, "%s Message %d", "My", 1);
}

TEST_F(LoggerTest, TestIsrFlagIsrMacroNoFormat)
{
    EXPECT_CALL(endpoint, Log(true, _, _)).Times(1);
    LogInit(LOG_LEVEL_INFO);
    LogAddEndpoint(LoggerProxy, &endpoint, LOG_LEVEL_INFO);
    LOG_ISR(LOG_LEVEL_FATAL, "Message");
}

TEST_F(LoggerTest, TestIsrFlagIsrMacroWithFormat)
{
    EXPECT_CALL(endpoint, Log(true, _, _)).Times(1);
    LogInit(LOG_LEVEL_INFO);
    LogAddEndpoint(LoggerProxy, &endpoint, LOG_LEVEL_INFO);
    LOGF_ISR(LOG_LEVEL_FATAL, "%s Message %d", "My", 1);
}

TEST_F(LoggerTest, TestIsrFlagImmediateMacroNoFormat)
{
    EXPECT_CALL(endpoint, Log(true, _, _)).Times(1);
    EXPECT_CALL(endpoint, Log(false, _, _)).Times(1);
    LogInit(LOG_LEVEL_INFO);
    LogAddEndpoint(LoggerProxy, &endpoint, LOG_LEVEL_INFO);
    LOGI(true, LOG_LEVEL_FATAL, "Message");
    LOGI(false, LOG_LEVEL_FATAL, "Message");
}

TEST_F(LoggerTest, TestIsrFlagImmediateMacroWithFormat)
{
    EXPECT_CALL(endpoint, Log(true, _, _)).Times(1);
    EXPECT_CALL(endpoint, Log(false, _, _)).Times(1);
    LogInit(LOG_LEVEL_INFO);
    LogAddEndpoint(LoggerProxy, &endpoint, LOG_LEVEL_INFO);
    LOGFI(false, LOG_LEVEL_FATAL, "%s Message %d", "My", 1);
    LOGFI(true, LOG_LEVEL_FATAL, "%s Message %d", "My", 1);
}
