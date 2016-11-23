#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "OsMock.hpp"
#include "base/os.h"
#include "time/timer.h"

using testing::Test;
using testing::NaggyMock;
using testing::Eq;
using testing::_;
using testing::Invoke;
using testing::Return;
using obc::time::TimeProvider;

class SmartWaitTest : public Test
{
  public:
    SmartWaitTest();

  protected:
    TimeProvider timeProvider;
    NaggyMock<OSMock> osMock;
    OSReset osGuard;
};

SmartWaitTest::SmartWaitTest()
{
    osGuard = InstallProxy(&osMock);

    EXPECT_CALL(osMock, TakeSemaphore(_, _)).WillRepeatedly(Return(OSResult::Success));
    EXPECT_CALL(osMock, GiveSemaphore(_)).WillRepeatedly(Return(OSResult::Success));
}

TEST_F(SmartWaitTest, ShouldReturnImmediatelyIfAlreadyAfterDesiredTime)
{
    EXPECT_CALL(osMock, PulseWait(_, _)).Times(0);

    timeProvider.CurrentTime = TimePointToTimeSpan(TimePointBuild(0, 0, 10, 0, 0));
    auto result = timeProvider.LongDelayUntil(TimePointBuild(0, 0, 10, 0, 0));

    ASSERT_THAT(result, Eq(true));
}

TEST_F(SmartWaitTest, ShouldWaitForPulseAndReturnIfDesiredTimeReached)
{
    timeProvider.CurrentTime = TimePointToTimeSpan(TimePointBuild(0, 0, 0, 0, 0));

    EXPECT_CALL(osMock, PulseWait(_, _))
        .Times(10)
        .WillRepeatedly(Invoke([&](OSPulseHandle handle, const OSTaskTimeSpan timeout) {
            UNUSED(handle, timeout);

            timeProvider.CurrentTime = TimeSpanAdd(timeProvider.CurrentTime, TimeSpanFromMinutes(1));
            return OSResult::Success;
        }));

    auto result = timeProvider.LongDelayUntil(TimePointBuild(0, 0, 10, 0, 0));

    ASSERT_THAT(result, Eq(true));
}

TEST_F(SmartWaitTest, ShouldWaitForPulseAndReturnIfMissionTimeJumpsOverDesiredTime)
{
    timeProvider.CurrentTime = TimePointToTimeSpan(TimePointBuild(0, 0, 0, 0, 0));

    EXPECT_CALL(osMock, PulseWait(_, _))
        .Times(11)
        .WillRepeatedly(Invoke([&](OSPulseHandle handle, const OSTaskTimeSpan timeout) {
            UNUSED(handle, timeout);

            timeProvider.CurrentTime = TimeSpanAdd(timeProvider.CurrentTime, TimeSpanFromMinutes(1));
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
