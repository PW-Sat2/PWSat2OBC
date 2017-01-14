#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "OsMock.hpp"
#include "base/os.h"
#include "mock/FsMock.hpp"
#include "time/timer.h"

using testing::Test;
using testing::NaggyMock;
using testing::Eq;
using testing::_;
using testing::Invoke;
using testing::Return;
using services::time::TimeProvider;
using namespace std::chrono_literals;

class SmartWaitTest : public Test
{
  public:
    SmartWaitTest();

  protected:
    TimeProvider timeProvider;
    NaggyMock<OSMock> osMock;
    testing::NiceMock<FsMock> fs;
    OSReset osGuard;
};

SmartWaitTest::SmartWaitTest() : timeProvider(fs)
{
    osGuard = InstallProxy(&osMock);

    EXPECT_CALL(osMock, TakeSemaphore(_, _)).WillRepeatedly(Return(OSResult::Success));
    EXPECT_CALL(osMock, GiveSemaphore(_)).WillRepeatedly(Return(OSResult::Success));

    ON_CALL(fs, Open(_, _, _)).WillByDefault(Return(MakeOpenedFile(1)));
    ON_CALL(fs, Write(_, _)).WillByDefault(Return(MakeFSIOResult(0)));
    ON_CALL(fs, Read(_, _)).WillByDefault(Return(MakeFSIOResult(0)));
}

TEST_F(SmartWaitTest, ShouldReturnImmediatelyIfAlreadyAfterDesiredTime)
{
    EXPECT_CALL(osMock, PulseWait(_, _)).Times(0);

    timeProvider.SetCurrentTime(TimePointBuild(0, 0, 10, 0, 0));
    auto result = timeProvider.LongDelayUntil(TimePointBuild(0, 0, 10, 0, 0));

    ASSERT_THAT(result, Eq(true));
}

TEST_F(SmartWaitTest, ShouldWaitForPulseAndReturnIfDesiredTimeReached)
{
    timeProvider.SetCurrentTime(TimePointBuild(0, 0, 0, 0, 0));

    EXPECT_CALL(osMock, PulseWait(_, _))
        .Times(10)
        .WillRepeatedly(Invoke([&](OSPulseHandle handle, const std::chrono::milliseconds timeout) {
            UNUSED(handle, timeout);

            Option<std::chrono::milliseconds> currentTime = timeProvider.GetCurrentTime();

            timeProvider.SetCurrentTime(TimePointFromDuration(currentTime.Value + 1min));
            return OSResult::Success;
        }));

    auto result = timeProvider.LongDelayUntil(TimePointBuild(0, 0, 10, 0, 0));

    ASSERT_THAT(result, Eq(true));
}

TEST_F(SmartWaitTest, ShouldWaitForPulseAndReturnIfMissionTimeJumpsOverDesiredTime)
{
    timeProvider.SetCurrentTime(TimePointBuild(0, 0, 0, 0, 0));

    EXPECT_CALL(osMock, PulseWait(_, _))
        .Times(11)
        .WillRepeatedly(Invoke([&](OSPulseHandle handle, const std::chrono::milliseconds timeout) {
            UNUSED(handle, timeout);

            Option<std::chrono::milliseconds> currentTime = timeProvider.GetCurrentTime();

            timeProvider.SetCurrentTime(TimePointFromDuration(currentTime.Value + 1min));
            return OSResult::Success;
        }));

    auto result = timeProvider.LongDelayUntil(TimePointBuild(0, 0, 10, 30, 0));

    ASSERT_THAT(result, Eq(true));
}

TEST_F(SmartWaitTest, ShouldAbortAfterWaitError)
{
    EXPECT_CALL(osMock, PulseWait(_, _)).WillOnce(Return(OSResult::NotSupported));

    auto result = timeProvider.LongDelay(TimeSpanFromDays(1));

    ASSERT_THAT(result, Eq(false));
}
