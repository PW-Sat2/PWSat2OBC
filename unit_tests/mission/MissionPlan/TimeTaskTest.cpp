#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "I2C/I2CMock.hpp"
#include "OsMock.hpp"
#include "mission/time.hpp"
#include "mock/FsMock.hpp"
#include "mock/NotifyTimeChangedMock.hpp"
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
namespace
{
    struct TimeTaskTest : public testing::Test
    {
        TimeTaskTest();

        testing::NiceMock<OSMock> mock;
        OSReset osReset;
        SystemState state;
        TimeProvider provider;
        RtcMock rtc;
        mission::TimeTask timeTask;
        mission::UpdateDescriptor<SystemState> updateDescriptor;
        mission::ActionDescriptor<SystemState> actionDescriptor;
        NotifyTimeChangedMock dummyMissionLoop;

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

        void SetPersistentState(milliseconds missionTime, milliseconds rtcTime)
        {
            state.PersistentState.Set(state::TimeState(missionTime, rtcTime));
        }
    };

    TimeTaskTest::TimeTaskTest()
        : timeTask(std::tie(provider, rtc, dummyMissionLoop)), //
          updateDescriptor(timeTask.BuildUpdate()),            //
          actionDescriptor(timeTask.BuildAction())
    {
        osReset = InstallProxy(&mock);

        ON_CALL(mock, CreateBinarySemaphore(_)).WillByDefault(Return(reinterpret_cast<OSSemaphoreHandle>(1)));

        provider.Initialize(0s, nullptr, nullptr);
    }

    TEST_F(TimeTaskTest, TestTimeUpdate)
    {
        ASSERT_TRUE(provider.SetCurrentTime(12345678s));

        EXPECT_CALL(mock, TakeSemaphore(_, _)).WillOnce(Return(OSResult::Success));
        const auto result = updateDescriptor.updateProc(state, updateDescriptor.param);
        ASSERT_THAT(result, Eq(UpdateResult::Ok));
        ASSERT_THAT(state.Time, Eq(12345678s));
    }

    TEST_F(TimeTaskTest, TestTimeUpdateFailure)
    {
        ASSERT_TRUE(provider.SetCurrentTime(12345678s));

        EXPECT_CALL(mock, TakeSemaphore(_, _)).WillOnce(Return(OSResult::IOError));
        const auto result = updateDescriptor.updateProc(state, updateDescriptor.param);
        ASSERT_THAT(result, Eq(UpdateResult::Warning));
        ASSERT_THAT(state.Time, Ne(12345678s));
    }

    TEST_F(TimeTaskTest, TestCorrectConditionBeforeTimeCorrectionPeriod)
    {
        // given
        // The correction was less than TimeCorrectionPeriod ago
        SetCurrentTime(0ms);
        SetPersistentState(0ms, rtc.GetTime());

        // then
        // CorrectTimeCondition returns false
        AdvanceTime(mission::TimeTask::TimeCorrectionPeriod - 1s);
        ASSERT_FALSE(actionDescriptor.EvaluateCondition(state));
    }

    TEST_F(TimeTaskTest, TestCorrectConditionAfterTimeCorrectionPeriod)
    {
        // given
        // The correction was at least TimeCorrectionPeriod ago
        SetCurrentTime(0ms);
        SetPersistentState(0ms, rtc.GetTime());

        // then
        // CorrectTimeCondition returns true
        AdvanceTime(mission::TimeTask::TimeCorrectionPeriod + 1s);
        ASSERT_TRUE(actionDescriptor.EvaluateCondition(state));
    }

    TEST_F(TimeTaskTest, TestCorrectionWithTimeUpdate)
    {
        // given
        // The correct action was run initially
        SetCurrentTime(0ms);
        SetPersistentState(0ms, rtc.GetTime());
        EXPECT_CALL(dummyMissionLoop, NotifyTimeChanged(Eq(-4s))).Times(1);

        // when
        // MCU runs much faster than RTC
        // And the correct action is run after at least TimeCorrectionPeriod

        AdvanceTime(mission::TimeTask::TimeCorrectionPeriod + 8s);
        rtc.AdvanceTime(duration_cast<seconds>(mission::TimeTask::TimeCorrectionPeriod));
        actionDescriptor.Execute(state);

        // then
        // Time provider should be updated
        auto expected = mission::TimeTask::TimeCorrectionPeriod + 4s;
        ASSERT_EQ(expected, provider.GetCurrentTime().Value);
        state::TimeState persistentTime;
        state.PersistentState.Get(persistentTime);
        ASSERT_THAT(persistentTime.LastMissionTime(), Eq(expected));
        ASSERT_THAT(persistentTime.LastExternalTime(), Eq(rtc.GetTime()));
    }

    TEST_F(TimeTaskTest, TestCorrectionWithTimeUpdateCustomCorrectionWeights)
    {
        // given
        // The correct action was run initially
        SetCurrentTime(0ms);
        SetPersistentState(0ms, rtc.GetTime());
        state.PersistentState.Set(state::TimeCorrectionConfiguration(1, 3));
        EXPECT_CALL(dummyMissionLoop, NotifyTimeChanged(Eq(-6000ms))).Times(1);

        // when
        // MCU runs much faster than RTC
        // And the correct action is run after at least TimeCorrectionPeriod

        AdvanceTime(mission::TimeTask::TimeCorrectionPeriod + 8s);
        rtc.AdvanceTime(duration_cast<seconds>(mission::TimeTask::TimeCorrectionPeriod));
        actionDescriptor.Execute(state);

        // then
        // Time provider should be updated
        auto expected = mission::TimeTask::TimeCorrectionPeriod + 2s;
        ASSERT_EQ(expected, provider.GetCurrentTime().Value);
        state::TimeState persistentTime;
        state.PersistentState.Get(persistentTime);
        ASSERT_THAT(persistentTime.LastMissionTime(), Eq(expected));
        ASSERT_THAT(persistentTime.LastExternalTime(), Eq(rtc.GetTime()));
    }

    TEST_F(TimeTaskTest, TestCorrectionWithTimeUpdate_RTCFaster)
    {
        // given
        // The correct action was run initially
        SetCurrentTime(0ms);
        SetPersistentState(0ms, rtc.GetTime());
        EXPECT_CALL(dummyMissionLoop, NotifyTimeChanged(Eq(4s))).Times(1);

        // when
        // MCU runs much slower than RTC
        // And the correct action is run after at least TimeCorrectionPeriod
        AdvanceTime(mission::TimeTask::TimeCorrectionPeriod);
        rtc.AdvanceTime(duration_cast<seconds>(mission::TimeTask::TimeCorrectionPeriod + 8s));
        actionDescriptor.Execute(state);

        // then
        // Time provider should be updated
        auto expected = mission::TimeTask::TimeCorrectionPeriod + 4s;
        ASSERT_EQ(expected, provider.GetCurrentTime().Value);
        state::TimeState persistentTime;
        state.PersistentState.Get(persistentTime);
        ASSERT_THAT(persistentTime.LastMissionTime(), Eq(expected));
        ASSERT_THAT(persistentTime.LastExternalTime(), Eq(rtc.GetTime()));
    }

    TEST_F(TimeTaskTest, TestTwoCorrectionsInRow)
    {
        // given
        // The correct action was run initially
        SetCurrentTime(0ms);
        SetPersistentState(0ms, rtc.GetTime());
        EXPECT_CALL(dummyMissionLoop, NotifyTimeChanged(_)).Times(2);

        // when
        // MCU runs much slower than RTC
        auto mcuDelta = mission::TimeTask::TimeCorrectionPeriod;
        auto rtcDelta = duration_cast<seconds>(mission::TimeTask::TimeCorrectionPeriod + 8s);

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

    TEST_F(TimeTaskTest, CorrectionDoesNotRunWhenRTCReadFails)
    {
        // given
        // The correct action was run initially
        SetCurrentTime(0ms);
        SetPersistentState(0ms, rtc.GetTime());
        EXPECT_CALL(dummyMissionLoop, NotifyTimeChanged(_)).Times(0);

        // when
        // MCU runs much faster than RTC
        // And the correct action is run after at least TimeCorrectionPeriod
        AdvanceTime(mission::TimeTask::TimeCorrectionPeriod);
        rtc.AdvanceTime(duration_cast<seconds>(mission::TimeTask::TimeCorrectionPeriod + 8s));

        // And the RTC cannot be read
        rtc.SetReadResult(OSResult::IOError);

        actionDescriptor.Execute(state);

        // then
        // Time provider should not be updated
        ASSERT_EQ(mission::TimeTask::TimeCorrectionPeriod, provider.GetCurrentTime().Value);
    }

    TEST_F(TimeTaskTest, CorrectionDoesNotRunWhenRTCReadsInvalidData)
    {
        // given
        // The correct action was run initially
        SetCurrentTime(0ms);
        SetPersistentState(0ms, rtc.GetTime());
        EXPECT_CALL(dummyMissionLoop, NotifyTimeChanged(_)).Times(0);

        // when
        // RTC reads invalid data
        devices::rtc::RTCTime invalidTime;
        invalidTime.minutes = 128;
        rtc.SetTime(invalidTime);

        // And the correct action is run after at least TimeCorrectionPeriod
        AdvanceTime(mission::TimeTask::TimeCorrectionPeriod);

        actionDescriptor.Execute(state);

        // then
        // Time provider should not be updated
        ASSERT_EQ(mission::TimeTask::TimeCorrectionPeriod, provider.GetCurrentTime().Value);
    }

    TEST_F(TimeTaskTest, CorrectionDoesNotRunWhenDeltaRTCIsZero)
    {
        // given
        // The correct action was run initially
        SetCurrentTime(0ms);
        SetPersistentState(0ms, rtc.GetTime());
        EXPECT_CALL(dummyMissionLoop, NotifyTimeChanged(Eq(std::chrono::milliseconds::zero()))).Times(1);

        // when
        // RTC time doesn't changed
        rtc.SetTime(rtc.GetTime());

        // And the correct action is run after at least TimeCorrectionPeriod
        AdvanceTime(mission::TimeTask::TimeCorrectionPeriod);

        actionDescriptor.Execute(state);

        // then
        // Time provider should not be updated
        ASSERT_EQ(mission::TimeTask::TimeCorrectionPeriod, provider.GetCurrentTime().Value);
    }

    TEST_F(TimeTaskTest, CorrectionDoesNotRunWhenTimeProviderReadFails)
    {
        // given
        // The correct action was run initially
        SetCurrentTime(0ms);
        SetPersistentState(0ms, rtc.GetTime());
        EXPECT_CALL(dummyMissionLoop, NotifyTimeChanged(_)).Times(0);

        // when
        // MCU runs much faster than RTC
        // And the correct action is run after at least TimeCorrectionPeriod
        AdvanceTime(mission::TimeTask::TimeCorrectionPeriod);
        rtc.AdvanceTime(duration_cast<seconds>(mission::TimeTask::TimeCorrectionPeriod + 8s));

        // And the correction action will fail to read time from TimeProvider
        EXPECT_CALL(mock, TakeSemaphore(_, _)).WillOnce(Return(OSResult::IOError)).WillRepeatedly(Return(OSResult::Success));

        actionDescriptor.Execute(state);

        // then
        // Time provider should not be updated.
        ASSERT_EQ(mission::TimeTask::TimeCorrectionPeriod, provider.GetCurrentTime().Value);
    }

    TEST_F(TimeTaskTest, TestCorrectionWithMaximumTimeValue)
    {
        // given
        // The correct action was run initially with maximum time value
        SetCurrentTime(milliseconds::max());
        SetPersistentState(milliseconds::max(), rtc.GetTime());
        EXPECT_CALL(dummyMissionLoop, NotifyTimeChanged(_)).Times(1);

        // when
        // MCU runs much slower than RTC
        // And the correct action is run after at least TimeCorrectionPeriod
        AdvanceTime(mission::TimeTask::TimeCorrectionPeriod);
        rtc.AdvanceTime(duration_cast<seconds>(mission::TimeTask::TimeCorrectionPeriod + 8s));
        actionDescriptor.Execute(state);

        // then
        // Time provider should be updated with overflown value
        auto expected = milliseconds::min() + mission::TimeTask::TimeCorrectionPeriod + 4s - 1ms;
        ASSERT_EQ(expected.count(), provider.GetCurrentTime().Value.count());
    }

    TEST_F(TimeTaskTest, CorrectionDoesNotRunWhenMaximumCorrectionThresholdReached)
    {
        // given
        // The correct action was run initially with maximum time value
        SetCurrentTime(0ms);
        SetPersistentState(0ms, rtc.GetTime());
        EXPECT_CALL(dummyMissionLoop, NotifyTimeChanged(Eq(0ms))).Times(1);

        // when
        // RTC reports very big time difference
        AdvanceTime(mission::TimeTask::TimeCorrectionPeriod);
        rtc.AdvanceTime(duration_cast<seconds>(mission::TimeTask::TimeCorrectionPeriod + 2 * mission::TimeTask::MaximumTimeCorrection));
        actionDescriptor.Execute(state);

        // then
        // Time provider should not be updated
        ASSERT_EQ(mission::TimeTask::TimeCorrectionPeriod, provider.GetCurrentTime().Value);
        state::TimeState persistentTime;
        state.PersistentState.Get(persistentTime);
        ASSERT_THAT(persistentTime.LastMissionTime(), Eq(mission::TimeTask::TimeCorrectionPeriod));
        ASSERT_THAT(persistentTime.LastExternalTime(), Eq(rtc.GetTime()));
    }

    TEST_F(TimeTaskTest, CorrectionRunsAsNormalAfterMaximumCorrectionThresholdReached)
    {
        {
            testing::InSequence s;
            EXPECT_CALL(dummyMissionLoop, NotifyTimeChanged(Eq(0ms))).Times(2);
            EXPECT_CALL(dummyMissionLoop, NotifyTimeChanged(Eq(4000ms))).Times(1);
        }

        // given
        // The correct action was run initially with maximum time value
        SetCurrentTime(0ms);
        actionDescriptor.Execute(state);

        // when
        // RTC reports very big time difference.
        AdvanceTime(mission::TimeTask::TimeCorrectionPeriod);
        rtc.AdvanceTime(duration_cast<seconds>(mission::TimeTask::TimeCorrectionPeriod + 2 * mission::TimeTask::MaximumTimeCorrection));

        actionDescriptor.Execute(state);

        // Another time correction is run with normal rtc operation.
        AdvanceTime(mission::TimeTask::TimeCorrectionPeriod);
        rtc.AdvanceTime(duration_cast<seconds>(mission::TimeTask::TimeCorrectionPeriod + 8s));

        actionDescriptor.Execute(state);

        // then
        // Time provider should be updated second time.
        auto expected = 2 * mission::TimeTask::TimeCorrectionPeriod + 4s;

        ASSERT_EQ(expected, provider.GetCurrentTime().Value);
    }

    TEST_F(TimeTaskTest, TimeShouldBeSavedWhenRTCIsDead)
    {
        SetCurrentTime(1h);
        SetPersistentState(45min, 67min);
        rtc.SetReadResult(OSResult::Busy);

        actionDescriptor.Execute(state);

        state::TimeState persistentTime;
        state.PersistentState.Get(persistentTime);
        ASSERT_THAT(persistentTime.LastMissionTime(), Eq(1h));
        ASSERT_THAT(persistentTime.LastExternalTime(), Eq(67min));
    }
}
