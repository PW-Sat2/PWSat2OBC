#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "OsMock.hpp"
#include "base/os.h"
//#include "os.hpp"
#include "time/timer.h"

using testing::Test;
using testing::NiceMock;
using testing::Eq;
using testing::_;
using testing::Invoke;

class SmartWaitTest : public Test
{
  public:
    SmartWaitTest();

  protected:
    TimeProvider timeProvider;
    NiceMock<OSMock> osMock;
    OSReset osGuard;
};

SmartWaitTest::SmartWaitTest()
{
    osGuard = InstallProxy(&osMock);
}

TEST_F(SmartWaitTest, ShouldReturnImmediatelyIfAlreadyAfterDesiredTime)
{
    timeProvider.CurrentTime = TimePointToTimeSpan(TimePointBuild(0, 0, 10, 0, 0));
    auto result = TimeLongDelayUntil(&timeProvider, TimePointBuild(0, 0, 10, 0, 0));

    ASSERT_THAT(result, Eq(true));
}

TEST_F(SmartWaitTest, ShouldWaitForPulseAndReturnIfDesiredTimeReached)
{
    timeProvider.CurrentTime = TimePointToTimeSpan(TimePointBuild(0, 0, 0, 0, 0));

    EXPECT_CALL(osMock, EventGroupWaitForBits(_, _, _, _, _))
        .WillRepeatedly(Invoke([&](
            OSEventGroupHandle eventGroup, const OSEventBits bitsToWaitFor, bool waitAll, bool autoReset, const OSTaskTimeSpan timeout) {
            UNUSED(eventGroup, bitsToWaitFor, waitAll, autoReset, timeout);

            timeProvider.CurrentTime = TimeSpanAdd(timeProvider.CurrentTime, TimeSpanFromMinutes(1));
            return (OSEventBits)0;
        }));

    auto result = TimeLongDelayUntil(&timeProvider, TimePointBuild(0, 0, 10, 0, 0));

    ASSERT_THAT(result, Eq(true));
}
