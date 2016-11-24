#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gmock/gmock-matchers.h"
#include "OsMock.hpp"
#include "TimeSpan.hpp"
#include "os/os.hpp"

#include "time/timer.h"

using testing::Eq;
using testing::Ne;
using testing::_;
using testing::Return;

struct TimeNotificationHandler
{
    MOCK_METHOD1(OnTimePassed, void(TimePoint point));
};

void TimePassedProxy(void* context, TimePoint currentTime)
{
    const auto handler = static_cast<TimeNotificationHandler*>(context);
    handler->OnTimePassed(currentTime);
}

class TimerTest : public testing::Test
{
  protected:
    void Initialize();

    TimeSpan GetCurrentTime();

    TimePoint GetMissionTime();

    TimeProvider provider;
    TimeNotificationHandler timeHandler;
    testing::NiceMock<OSMock> os;
    OSReset guard;
};

void TimerTest::Initialize()
{
    this->guard = InstallProxy(&os);
    EXPECT_CALL(os, CreateBinarySemaphore()).WillOnce(Return(reinterpret_cast<void*>(1))).WillOnce(Return(reinterpret_cast<void*>(2)));
    ON_CALL(os, TakeSemaphore(_, _)).WillByDefault(Return(OSResult::Success));
    ON_CALL(os, GiveSemaphore(_)).WillByDefault(Return(OSResult::Success));

    EXPECT_CALL(os, CreatePulseAll()).WillOnce(Return(reinterpret_cast<void*>(3)));

    EXPECT_TRUE(TimeInitialize(&provider, TimePassedProxy, &timeHandler, nullptr));
}

TimeSpan TimerTest::GetCurrentTime()
{
    TimeSpan span{0};
    EXPECT_CALL(os, TakeSemaphore(provider.timerLock, _)).WillOnce(Return(OSResult::Success));
    EXPECT_CALL(os, GiveSemaphore(provider.timerLock)).Times(1);
    EXPECT_TRUE(TimeGetCurrentTime(&provider, &span));
    return span;
}

TimePoint TimerTest::GetMissionTime()
{
    TimePoint point{0, 0, 0, 0, 0};
    EXPECT_CALL(os, TakeSemaphore(provider.timerLock, _)).WillOnce(Return(OSResult::Success));
    EXPECT_CALL(os, GiveSemaphore(provider.timerLock)).Times(1);
    EXPECT_TRUE(TimeGetCurrentMissionTime(&provider, &point));
    return point;
}

TEST_F(TimerTest, TestDefaultState)
{
    this->guard = InstallProxy(&os);
    ON_CALL(os, CreateBinarySemaphore()).WillByDefault(Return(reinterpret_cast<void*>(this)));
    ON_CALL(os, CreatePulseAll()).WillByDefault(Return(reinterpret_cast<void*>(3)));
    const auto result = TimeInitialize(&provider, TimePassedProxy, &timeHandler, nullptr);
    ASSERT_THAT(result, Eq(true));
    ASSERT_THAT(GetCurrentTime(), Eq(TimeSpanFromMilliseconds(0ull)));
    const auto time = GetMissionTime();
    ASSERT_THAT(time.milisecond, Eq(0));
    ASSERT_THAT(time.second, Eq(0));
    ASSERT_THAT(time.minute, Eq(0));
    ASSERT_THAT(time.hour, Eq(0));
    ASSERT_THAT(time.day, Eq(0));
}

TEST_F(TimerTest, TestInitializationFailure)
{
    ON_CALL(os, CreateBinarySemaphore()).WillByDefault(Return(nullptr));
    const auto result = TimeInitialize(&provider, TimePassedProxy, &timeHandler, nullptr);
    ASSERT_THAT(result, Eq(false));
}

TEST_F(TimerTest, TestInitializationSecondFailure)
{
    this->guard = InstallProxy(&os);
    EXPECT_CALL(os, CreateBinarySemaphore()).WillOnce(Return(reinterpret_cast<void*>(this))).WillOnce(Return(nullptr));
    const auto result = TimeInitialize(&provider, TimePassedProxy, &timeHandler, nullptr);
    ASSERT_THAT(result, Eq(false));
}

TEST_F(TimerTest, TestAdvanceTimeNoNotification)
{
    Initialize();
    EXPECT_CALL(timeHandler, OnTimePassed(_)).Times(0);
    TimeAdvanceTime(&provider, TimeSpanFromMilliseconds(1ull));
}

TEST_F(TimerTest, TestAdvanceTimeWithNotification)
{
    EXPECT_CALL(timeHandler, OnTimePassed(_)).Times(1);
    Initialize();
    TimeAdvanceTime(&provider, TimeSpanFromMilliseconds(10001ull));
}

TEST_F(TimerTest, TestAdvanceTimeWithNotificationCheckNotificationSemaphores)
{
    Initialize();
    EXPECT_CALL(os, TakeSemaphore(provider.timerLock, _)).WillOnce(Return(OSResult::Success));
    EXPECT_CALL(os, GiveSemaphore(provider.timerLock)).Times(1);
    EXPECT_CALL(os, TakeSemaphore(provider.notificationLock, _)).WillOnce(Return(OSResult::Success));
    EXPECT_CALL(os, GiveSemaphore(provider.notificationLock)).Times(1);
    TimeAdvanceTime(&provider, TimeSpanFromMilliseconds(10001ull));
}

TEST_F(TimerTest, TestAdvanceTimeWithNotificationLockFailure)
{
    EXPECT_CALL(timeHandler, OnTimePassed(_)).Times(0);
    Initialize();
    EXPECT_CALL(os, TakeSemaphore(provider.timerLock, _)).WillOnce(Return(OSResult::Success));
    EXPECT_CALL(os, GiveSemaphore(provider.timerLock)).Times(1);
    EXPECT_CALL(os, TakeSemaphore(provider.notificationLock, _)).WillOnce(Return(OSResult::InvalidOperation));
    EXPECT_CALL(os, GiveSemaphore(provider.notificationLock)).Times(0);
    TimeAdvanceTime(&provider, TimeSpanFromMilliseconds(10001ull));
    ASSERT_THAT(GetCurrentTime(), Eq(TimeSpanFromMilliseconds(10001u)));
}

TEST_F(TimerTest, TestAdvanceTimeWithTimerLockFailure)
{
    Initialize();
    EXPECT_CALL(os, TakeSemaphore(provider.timerLock, _)).WillOnce(Return(OSResult::InvalidOperation));
    EXPECT_CALL(os, GiveSemaphore(provider.timerLock)).Times(0);
    TimeAdvanceTime(&provider, TimeSpanFromMilliseconds(500ull));
    ASSERT_THAT(GetCurrentTime(), Eq(TimeSpanFromMilliseconds(0u)));
}

TEST_F(TimerTest, TestCurrentTimeAfterJump)
{
    Initialize();
    TimeAdvanceTime(&provider, TimeSpanFromMilliseconds(10001ull));
    ASSERT_THAT(GetCurrentTime(), Eq(TimeSpanFromMilliseconds(10001u)));
}

TEST_F(TimerTest, TestMultipleJumps)
{
    EXPECT_CALL(timeHandler, OnTimePassed(_)).Times(1);
    Initialize();
    TimeAdvanceTime(&provider, TimeSpanFromMilliseconds(1000ull));
    TimeAdvanceTime(&provider, TimeSpanFromMilliseconds(3500ull));
    TimeAdvanceTime(&provider, TimeSpanFromMilliseconds(2000ull));
    ASSERT_THAT(GetCurrentTime(), Eq(TimeSpanFromMilliseconds(6500u)));
}

TEST_F(TimerTest, TestSetTime)
{
    Initialize();
    TimePoint point;
    point.milisecond = 1;
    point.second = 2;
    point.minute = 3;
    point.hour = 4;
    point.day = 5;
    const auto status = TimeSetCurrentTime(&provider, point);
    ASSERT_THAT(status, Eq(true));
    ASSERT_THAT(GetCurrentTime(), Eq(TimeSpanFromMilliseconds(446582001u)));
}

TEST_F(TimerTest, TestSetTimeLockError)
{
    Initialize();
    EXPECT_CALL(os, TakeSemaphore(provider.timerLock, _)).WillOnce(Return(OSResult::InvalidOperation));
    EXPECT_CALL(os, GiveSemaphore(provider.timerLock)).Times(0);
    TimePoint point;
    point.milisecond = 1;
    point.second = 2;
    point.minute = 3;
    point.hour = 4;
    point.day = 5;
    const auto status = TimeSetCurrentTime(&provider, point);
    ASSERT_THAT(status, Eq(false));
    ASSERT_THAT(GetCurrentTime(), Ne(TimeSpanFromMilliseconds(446582001u)));
}

TEST_F(TimerTest, TestSetTimeNotifiesSystemAboutTimeChange)
{
    Initialize();
    TimePoint point;
    point.milisecond = 1;
    point.second = 2;
    point.minute = 3;
    point.hour = 4;
    point.day = 5;
    EXPECT_CALL(timeHandler, OnTimePassed(_)).Times(1);
    TimeSetCurrentTime(&provider, point);
}

TEST_F(TimerTest, TestGetCurrentMissionTime)
{
    TimePoint point{0, 0, 0, 0, 0};
    Initialize();
    TimeAdvanceTime(&provider, TimeSpanFromMilliseconds(446582001ull));
    ASSERT_TRUE(TimeGetCurrentMissionTime(&provider, &point));
    ASSERT_THAT(point.milisecond, Eq(1));
    ASSERT_THAT(point.second, Eq(2));
    ASSERT_THAT(point.minute, Eq(3));
    ASSERT_THAT(point.hour, Eq(4));
    ASSERT_THAT(point.day, Eq(5));
}

TEST_F(TimerTest, TestGetCurrentMissionTimeSynchronization)
{
    TimePoint point{0, 0, 0, 0, 0};
    Initialize();
    TimeAdvanceTime(&provider, TimeSpanFromMilliseconds(446582001ull));
    EXPECT_CALL(os, TakeSemaphore(provider.timerLock, _)).WillOnce(Return(OSResult::Success));
    EXPECT_CALL(os, GiveSemaphore(provider.timerLock)).Times(1);
    TimeGetCurrentMissionTime(&provider, &point);
}

TEST_F(TimerTest, TestGetCurrentMissionTimeSynchronizationFailure)
{
    TimePoint point{0, 0, 0, 0, 0};
    Initialize();
    TimeAdvanceTime(&provider, TimeSpanFromMilliseconds(446582001ull));
    EXPECT_CALL(os, TakeSemaphore(provider.timerLock, _)).WillOnce(Return(OSResult::InvalidOperation));
    EXPECT_CALL(os, GiveSemaphore(provider.timerLock)).Times(0);
    EXPECT_FALSE(TimeGetCurrentMissionTime(&provider, &point));
}

TEST_F(TimerTest, TestGetCurrentTimeSynchronization)
{
    TimeSpan span{0};
    Initialize();
    TimeAdvanceTime(&provider, TimeSpanFromMilliseconds(446582001ull));
    EXPECT_CALL(os, TakeSemaphore(provider.timerLock, _)).WillOnce(Return(OSResult::Success));
    EXPECT_CALL(os, GiveSemaphore(provider.timerLock)).Times(1);
    TimeGetCurrentTime(&provider, &span);
}

TEST_F(TimerTest, TestGetCurrentTimeSynchronizationFailure)
{
    TimeSpan span{0};
    Initialize();
    TimeAdvanceTime(&provider, TimeSpanFromMilliseconds(446582001ull));
    EXPECT_CALL(os, TakeSemaphore(provider.timerLock, _)).WillOnce(Return(OSResult::InvalidOperation));
    EXPECT_CALL(os, GiveSemaphore(provider.timerLock)).Times(0);
    EXPECT_FALSE(TimeGetCurrentTime(&provider, &span));
}

TEST_F(TimerTest, TestTimerCallback)
{
    Initialize();
    EXPECT_CALL(timeHandler, OnTimePassed(_)).Times(1);
    auto callback = TimeGetTickProcedure();
    callback(&provider, TimeSpanFromMilliseconds(446582001ull));
    const auto point = GetMissionTime();
    ASSERT_THAT(point.milisecond, Eq(1));
    ASSERT_THAT(point.second, Eq(2));
    ASSERT_THAT(point.minute, Eq(3));
    ASSERT_THAT(point.hour, Eq(4));
    ASSERT_THAT(point.day, Eq(5));
}
