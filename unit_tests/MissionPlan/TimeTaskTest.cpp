#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "I2C/I2CMock.hpp"
#include "OsMock.hpp"
#include "mission/time.hpp"
#include "mock/FsMock.hpp"
#include "mock/RtcMock.hpp"
#include "os/os.hpp"
#include "time/TimeSpan.hpp"
#include "time/timer.h"

using testing::Eq;
using testing::Ne;
using testing::Return;
using testing::_;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;
using namespace mission;
using namespace std::chrono_literals;

struct TimeTaskTest : public testing::Test
{
    TimeTaskTest();

    OSMock mock;
    FsMock fileSystemMock;
    SystemState state;
    TimeProvider provider;
    RtcMock rtc;
    mission::TimeTask timeTask;
    mission::UpdateDescriptor<SystemState> updateDescriptor;
    mission::ActionDescriptor<SystemState> actionDescriptor;

    void SetCurrentTime(milliseconds time)
    {
        ASSERT_TRUE(provider.SetCurrentTime(time));
        state.Time = time;
    }

    void AdvanceTime(milliseconds delta)
    {
        auto newTime = provider.GetCurrentTime().Value + delta;
        SetCurrentTime(newTime);
    }
};

TimeTaskTest::TimeTaskTest()
    : provider(fileSystemMock),                 //
      timeTask(std::tie(provider, rtc)),        //
      updateDescriptor(timeTask.BuildUpdate()), //
      actionDescriptor(timeTask.BuildAction())
{
    ON_CALL(fileSystemMock, Open(_, _, _)).WillByDefault(Return(MakeOpenedFile(1)));
    ON_CALL(fileSystemMock, Write(_, _)).WillByDefault(Return(MakeFSIOResult(0)));
    ON_CALL(fileSystemMock, Read(_, _)).WillByDefault(Return(MakeFSIOResult(0)));
}

TEST_F(TimeTaskTest, TestTimeUpdate)
{
    auto proxy = InstallProxy(&mock);
    ASSERT_TRUE(provider.SetCurrentTime(12345678s));

    EXPECT_CALL(mock, TakeSemaphore(_, _)).WillOnce(Return(OSResult::Success));
    const auto result = updateDescriptor.updateProc(state, updateDescriptor.param);
    ASSERT_THAT(result, Eq(UpdateResult::Ok));
    ASSERT_THAT(state.Time, Eq(12345678s));
}

TEST_F(TimeTaskTest, TestTimeUpdateFailure)
{
    auto proxy = InstallProxy(&mock);
    ASSERT_TRUE(provider.SetCurrentTime(12345678s));

    EXPECT_CALL(mock, TakeSemaphore(_, _)).WillOnce(Return(OSResult::IOError));
    const auto result = updateDescriptor.updateProc(state, updateDescriptor.param);
    ASSERT_THAT(result, Eq(UpdateResult::Warning));
    ASSERT_THAT(state.Time, Ne(12345678s));
}

TEST_F(TimeTaskTest, TestInitialCorrectCondition)
{
    // given The correction was not run
    // then correction condition returns true
    ASSERT_TRUE(actionDescriptor.EvaluateCondition(state));
}

TEST_F(TimeTaskTest, TestCorrectConditionBeforeTimeCorrectionPeriod)
{
    auto proxy = InstallProxy(&mock);

    // given
    // The correction was less than TimeCorrectionPeriod ago
    SetCurrentTime(0ms);
    actionDescriptor.Execute(state);

    // then
    // CorrectTimeCondition returns false
    AdvanceTime(mission::TimeCorrectionPeriod - 1s);
    ASSERT_FALSE(actionDescriptor.EvaluateCondition(state));
}

TEST_F(TimeTaskTest, TestCorrectConditionAfterTimeCorrectionPeriod)
{
    auto proxy = InstallProxy(&mock);

    // given
    // The correction was at least TimeCorrectionPeriod ago
    SetCurrentTime(0ms);
    actionDescriptor.Execute(state);

    // then
    // CorrectTimeCondition returns true
    AdvanceTime(mission::TimeCorrectionPeriod + 1s);
    ASSERT_TRUE(actionDescriptor.EvaluateCondition(state));
}

TEST_F(TimeTaskTest, TestCorrectionWithTimeUpdate)
{
    auto proxy = InstallProxy(&mock);

    // given
    // The correct action was run initially
    SetCurrentTime(0ms);
    actionDescriptor.Execute(state);

    // when
    // MCU runs much faster than RTC
    // And the correct action is run after at least TimeCorrectionPeriod
    AdvanceTime(mission::TimeCorrectionPeriod + 4 * mission::MinimumTimeCorrection);
    rtc.AdvanceTime(duration_cast<seconds>(mission::TimeCorrectionPeriod));
    actionDescriptor.Execute(state);

    // then
    // Time provider should be updated
    auto expected = mission::TimeCorrectionPeriod + 2 * mission::MinimumTimeCorrection;
    ASSERT_EQ(expected, provider.GetCurrentTime().Value);
}

TEST_F(TimeTaskTest, TestCorrectionWithTimeUpdate_RTCFaster)
{
    auto proxy = InstallProxy(&mock);

    // given
    // The correct action was run initially
    SetCurrentTime(0ms);
    actionDescriptor.Execute(state);

    // when
    // MCU runs much slower than RTC
    // And the correct action is run after at least TimeCorrectionPeriod
    AdvanceTime(mission::TimeCorrectionPeriod);
    rtc.AdvanceTime(duration_cast<seconds>(mission::TimeCorrectionPeriod + 4 * mission::MinimumTimeCorrection));
    actionDescriptor.Execute(state);

    // then
    // Time provider should be updated
    auto expected = mission::TimeCorrectionPeriod + 2 * mission::MinimumTimeCorrection;
    ASSERT_EQ(expected, provider.GetCurrentTime().Value);
}

TEST_F(TimeTaskTest, TestTooSmallCorrection)
{
    auto proxy = InstallProxy(&mock);

    // given
    // The correct action was run initially
    SetCurrentTime(0ms);
    actionDescriptor.Execute(state);

    // when
    // MCU runs much slower than RTC
    // And the correct action is run after TimeCorrectionPeriod
    AdvanceTime(mission::TimeCorrectionPeriod);
    rtc.AdvanceTime(duration_cast<seconds>(mission::TimeCorrectionPeriod + mission::MinimumTimeCorrection / 2));
    actionDescriptor.Execute(state);

    // then
    // Time provider should not be updated
    auto expected = mission::TimeCorrectionPeriod;
    ASSERT_EQ(expected, provider.GetCurrentTime().Value);
}

TEST_F(TimeTaskTest, TestTwoCorrectionsInRow)
{
    auto proxy = InstallProxy(&mock);

    // given
    // The correct action was run initially
    SetCurrentTime(0ms);
    actionDescriptor.Execute(state);

    // when
    // MCU runs much slower than RTC
    auto mcuDelta = mission::TimeCorrectionPeriod;
    auto rtcDelta = duration_cast<seconds>(mission::TimeCorrectionPeriod + 4 * mission::MinimumTimeCorrection);

    // And auto correction is run twice
    AdvanceTime(mcuDelta);
    rtc.AdvanceTime(rtcDelta);
    actionDescriptor.Execute(state);

    auto correctedTime = provider.GetCurrentTime().Value;
    auto correctionDelta = correctedTime - mcuDelta;

    AdvanceTime(mcuDelta);
    rtc.AdvanceTime(rtcDelta);
    actionDescriptor.Execute(state);

    // then
    // Time provider should be updated
    auto expected = correctedTime + mcuDelta + correctionDelta;
    ASSERT_EQ(expected, provider.GetCurrentTime().Value);
}

TEST_F(TimeTaskTest, CorrectionDoesRunWhenRTCReadFails)
{
    auto proxy = InstallProxy(&mock);

    // given
    // The correct action was run initially
    SetCurrentTime(0ms);
    actionDescriptor.Execute(state);

    // when
    // MCU runs much faster than RTC
    // And the correct action is run after at least TimeCorrectionPeriod
    AdvanceTime(mission::TimeCorrectionPeriod);
    rtc.AdvanceTime(duration_cast<seconds>(mission::TimeCorrectionPeriod + 4 * mission::MinimumTimeCorrection));

    // And the RTC cannot be read
    rtc.SetReadResult(OSResult::IOError);

    actionDescriptor.Execute(state);

    // then
    // Time provider should not be updated
    ASSERT_EQ(mission::TimeCorrectionPeriod, provider.GetCurrentTime().Value);
}

TEST_F(TimeTaskTest, CorrectionDoesRunWhenTimeProviderReadFails)
{
    auto proxy = InstallProxy(&mock);

    // given
    // The correct action was run initially
    SetCurrentTime(0ms);
    actionDescriptor.Execute(state);

    // when
    // MCU runs much faster than RTC
    // And the correct action is run after at least TimeCorrectionPeriod
    AdvanceTime(mission::TimeCorrectionPeriod);
    rtc.AdvanceTime(duration_cast<seconds>(mission::TimeCorrectionPeriod + 4 * mission::MinimumTimeCorrection));

    // And the correction action will fail to read time from TimeProvider
    EXPECT_CALL(mock, TakeSemaphore(_, _)).WillOnce(Return(OSResult::IOError)).WillRepeatedly(Return(OSResult::Success));

    actionDescriptor.Execute(state);

    // then
    // Time provider should not be updated.
    ASSERT_EQ(mission::TimeCorrectionPeriod, provider.GetCurrentTime().Value);
}

TEST_F(TimeTaskTest, TestCorrectionWithMaximumTimeValue)
{
    auto proxy = InstallProxy(&mock);

    // given
    // The correct action was run initially with maximum time value
    SetCurrentTime(milliseconds::max());
    actionDescriptor.Execute(state);

    // when
    // MCU runs much slower than RTC
    // And the correct action is run after at least TimeCorrectionPeriod
    AdvanceTime(mission::TimeCorrectionPeriod);
    rtc.AdvanceTime(duration_cast<seconds>(mission::TimeCorrectionPeriod + 4 * mission::MinimumTimeCorrection));
    actionDescriptor.Execute(state);

    // then
    // Time provider should be updated with overflown value
    auto expected = milliseconds::min() + mission::TimeCorrectionPeriod + 2 * mission::MinimumTimeCorrection - 1ms;
    ASSERT_EQ(expected.count(), provider.GetCurrentTime().Value.count());
}
