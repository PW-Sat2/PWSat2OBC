#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gmock/gmock-matchers.h"
#include "OsMock.hpp"
#include "TimeSpan.hpp"
#include "mock/FsMock.hpp"
#include "os/os.hpp"

#include "time/timer.h"

using testing::Eq;
using testing::Ne;
using testing::_;
using testing::Return;
using services::time::TimeProvider;
using std::chrono::milliseconds;

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
    TimerTest();
    void Initialize();

    std::chrono::milliseconds GetCurrentTime();

    TimePoint GetMissionTime();

    TimeProvider provider;
    TimeNotificationHandler timeHandler;

    testing::NiceMock<FsMock> fs;
    testing::NiceMock<OSMock> os;
    OSReset guard;

    OSSemaphoreHandle timerLockHandle = reinterpret_cast<void*>(1);
    OSSemaphoreHandle notificationLockHandle = reinterpret_cast<void*>(2);
};

TimerTest::TimerTest() : provider(fs)
{
    ON_CALL(fs, Open(_, _, _)).WillByDefault(Return(MakeOpenedFile(1)));
    ON_CALL(fs, Write(_, _)).WillByDefault(Return(MakeFSIOResult(0)));
    ON_CALL(fs, Read(_, _)).WillByDefault(Return(MakeFSIOResult(0)));
}

void TimerTest::Initialize()
{
    this->guard = InstallProxy(&os);

    EXPECT_CALL(os, CreateBinarySemaphore(TimeProvider::TIMER_LOCK_ID)).WillOnce(Return(timerLockHandle));
    EXPECT_CALL(os, CreateBinarySemaphore(TimeProvider::NOTIFICATION_LOCK_ID)).WillOnce(Return(notificationLockHandle));

    ON_CALL(os, TakeSemaphore(_, _)).WillByDefault(Return(OSResult::Success));
    ON_CALL(os, GiveSemaphore(_)).WillByDefault(Return(OSResult::Success));

    EXPECT_CALL(os, CreatePulseAll()).WillOnce(Return(reinterpret_cast<void*>(3)));

    EXPECT_TRUE(provider.Initialize(TimePassedProxy, &timeHandler));
}

std::chrono::milliseconds TimerTest::GetCurrentTime()
{
    EXPECT_CALL(os, TakeSemaphore(timerLockHandle, _)).WillOnce(Return(OSResult::Success));
    EXPECT_CALL(os, GiveSemaphore(timerLockHandle)).Times(1);
    Option<std::chrono::milliseconds> span = provider.GetCurrentTime();
    EXPECT_TRUE(span.HasValue);
    return span.Value;
}

TimePoint TimerTest::GetMissionTime()
{
    EXPECT_CALL(os, TakeSemaphore(timerLockHandle, _)).WillOnce(Return(OSResult::Success));
    EXPECT_CALL(os, GiveSemaphore(timerLockHandle)).Times(1);
    Option<TimePoint> point = provider.GetCurrentMissionTime();
    EXPECT_TRUE(point.HasValue);
    return point.Value;
}

TEST_F(TimerTest, TestDefaultState)
{
    this->guard = InstallProxy(&os);
    EXPECT_CALL(os, CreateBinarySemaphore(TimeProvider::TIMER_LOCK_ID)).WillOnce(Return(timerLockHandle));
    EXPECT_CALL(os, CreateBinarySemaphore(TimeProvider::NOTIFICATION_LOCK_ID)).WillOnce(Return(notificationLockHandle));
    ON_CALL(os, CreatePulseAll()).WillByDefault(Return(reinterpret_cast<void*>(3)));
    const auto result = provider.Initialize(TimePassedProxy, &timeHandler);
    ASSERT_THAT(result, Eq(true));
    ASSERT_THAT(GetCurrentTime(), Eq(milliseconds(0ull)));
    const auto time = GetMissionTime();
    ASSERT_THAT(time.milisecond, Eq(0));
    ASSERT_THAT(time.second, Eq(0));
    ASSERT_THAT(time.minute, Eq(0));
    ASSERT_THAT(time.hour, Eq(0));
    ASSERT_THAT(time.day, Eq(0));
}

TEST_F(TimerTest, TestInitializationFailure)
{
    ON_CALL(os, CreateBinarySemaphore(_)).WillByDefault(Return(nullptr));
    const auto result = provider.Initialize(TimePassedProxy, &timeHandler);
    ASSERT_THAT(result, Eq(false));
}

TEST_F(TimerTest, TestInitializationSecondFailure)
{
    this->guard = InstallProxy(&os);
    EXPECT_CALL(os, CreateBinarySemaphore(_)).WillOnce(Return(reinterpret_cast<void*>(this))).WillOnce(Return(nullptr));
    const auto result = provider.Initialize(TimePassedProxy, &timeHandler);
    ASSERT_THAT(result, Eq(false));
}

TEST_F(TimerTest, TestAdvanceTimeNoNotification)
{
    Initialize();
    EXPECT_CALL(timeHandler, OnTimePassed(_)).Times(0);
    provider.AdvanceTime(milliseconds(1ull));
}

TEST_F(TimerTest, TestAdvanceTimeWithNotification)
{
    EXPECT_CALL(timeHandler, OnTimePassed(_)).Times(1);
    Initialize();
    provider.AdvanceTime(milliseconds(10001ull));
}

TEST_F(TimerTest, TestAdvanceTimeWithNotificationCheckNotificationSemaphores)
{
    Initialize();
    EXPECT_CALL(os, TakeSemaphore(timerLockHandle, _)).WillOnce(Return(OSResult::Success));
    EXPECT_CALL(os, GiveSemaphore(timerLockHandle)).Times(1);
    EXPECT_CALL(os, TakeSemaphore(notificationLockHandle, _)).WillOnce(Return(OSResult::Success));
    EXPECT_CALL(os, GiveSemaphore(notificationLockHandle)).Times(1);
    provider.AdvanceTime(milliseconds(10001ull));
}

TEST_F(TimerTest, TestAdvanceTimeWithNotificationLockFailure)
{
    EXPECT_CALL(timeHandler, OnTimePassed(_)).Times(0);
    Initialize();
    EXPECT_CALL(os, TakeSemaphore(timerLockHandle, _)).WillOnce(Return(OSResult::Success));
    EXPECT_CALL(os, GiveSemaphore(timerLockHandle)).Times(1);
    EXPECT_CALL(os, TakeSemaphore(notificationLockHandle, _)).WillOnce(Return(OSResult::InvalidOperation));
    EXPECT_CALL(os, GiveSemaphore(notificationLockHandle)).Times(0);
    provider.AdvanceTime(milliseconds(10001ull));
    ASSERT_THAT(GetCurrentTime(), Eq(milliseconds(10001u)));
}

TEST_F(TimerTest, TestAdvanceTimeWithTimerLockFailure)
{
    Initialize();
    EXPECT_CALL(os, TakeSemaphore(timerLockHandle, _)).WillOnce(Return(OSResult::InvalidOperation));
    EXPECT_CALL(os, GiveSemaphore(timerLockHandle)).Times(0);
    provider.AdvanceTime(milliseconds(500ull));
    ASSERT_THAT(GetCurrentTime(), Eq(milliseconds(0u)));
}

TEST_F(TimerTest, TestCurrentTimeAfterJump)
{
    Initialize();
    provider.AdvanceTime(milliseconds(10001ull));
    ASSERT_THAT(GetCurrentTime(), Eq(milliseconds(10001u)));
}

TEST_F(TimerTest, TestMultipleJumps)
{
    EXPECT_CALL(timeHandler, OnTimePassed(_)).Times(1);
    Initialize();
    provider.AdvanceTime(milliseconds(1000ull));
    provider.AdvanceTime(milliseconds(3500ull));
    provider.AdvanceTime(milliseconds(2000ull));
    ASSERT_THAT(GetCurrentTime(), Eq(milliseconds(6500u)));
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
    const auto status = provider.SetCurrentTime(point);
    ASSERT_THAT(status, Eq(true));
    ASSERT_THAT(GetCurrentTime(), Eq(milliseconds(446582001u)));
}

TEST_F(TimerTest, TestSetTimeLockError)
{
    Initialize();
    EXPECT_CALL(os, TakeSemaphore(timerLockHandle, _)).WillOnce(Return(OSResult::InvalidOperation));
    EXPECT_CALL(os, GiveSemaphore(timerLockHandle)).Times(0);
    TimePoint point;
    point.milisecond = 1;
    point.second = 2;
    point.minute = 3;
    point.hour = 4;
    point.day = 5;
    const auto status = provider.SetCurrentTime(point);
    ASSERT_THAT(status, Eq(false));
    ASSERT_THAT(GetCurrentTime(), Ne(milliseconds(446582001u)));
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
    provider.SetCurrentTime(point);
}

TEST_F(TimerTest, TestGetCurrentMissionTime)
{
    Initialize();
    provider.AdvanceTime(milliseconds(446582001ull));
    Option<TimePoint> point = provider.GetCurrentMissionTime();
    ASSERT_TRUE(point.HasValue);
    ASSERT_THAT(point.Value.milisecond, Eq(1));
    ASSERT_THAT(point.Value.second, Eq(2));
    ASSERT_THAT(point.Value.minute, Eq(3));
    ASSERT_THAT(point.Value.hour, Eq(4));
    ASSERT_THAT(point.Value.day, Eq(5));
}

TEST_F(TimerTest, TestGetCurrentMissionTimeSynchronization)
{
    Initialize();
    provider.AdvanceTime(milliseconds(446582001ull));
    EXPECT_CALL(os, TakeSemaphore(timerLockHandle, _)).WillOnce(Return(OSResult::Success));
    EXPECT_CALL(os, GiveSemaphore(timerLockHandle)).Times(1);

    provider.GetCurrentMissionTime();
}

TEST_F(TimerTest, TestGetCurrentMissionTimeSynchronizationFailure)
{
    Initialize();
    provider.AdvanceTime(milliseconds(446582001ull));
    EXPECT_CALL(os, TakeSemaphore(timerLockHandle, _)).WillOnce(Return(OSResult::InvalidOperation));
    EXPECT_CALL(os, GiveSemaphore(timerLockHandle)).Times(0);
    EXPECT_FALSE(provider.GetCurrentMissionTime().HasValue);
}

TEST_F(TimerTest, TestGetCurrentTimeSynchronization)
{
    Initialize();
    provider.AdvanceTime(milliseconds(446582001ull));
    EXPECT_CALL(os, TakeSemaphore(timerLockHandle, _)).WillOnce(Return(OSResult::Success));
    EXPECT_CALL(os, GiveSemaphore(timerLockHandle)).Times(1);
    provider.GetCurrentTime();
}

TEST_F(TimerTest, TestGetCurrentTimeSynchronizationFailure)
{
    Initialize();
    provider.AdvanceTime(milliseconds(446582001ull));
    EXPECT_CALL(os, TakeSemaphore(timerLockHandle, _)).WillOnce(Return(OSResult::InvalidOperation));
    EXPECT_CALL(os, GiveSemaphore(timerLockHandle)).Times(0);
    EXPECT_FALSE(provider.GetCurrentTime().HasValue);
}

TEST_F(TimerTest, TestTimerCallback)
{
    Initialize();
    EXPECT_CALL(timeHandler, OnTimePassed(_)).Times(1);
    provider.AdvanceTime(milliseconds(446582001ull));
    const auto point = GetMissionTime();
    ASSERT_THAT(point.milisecond, Eq(1));
    ASSERT_THAT(point.second, Eq(2));
    ASSERT_THAT(point.minute, Eq(3));
    ASSERT_THAT(point.hour, Eq(4));
    ASSERT_THAT(point.day, Eq(5));
}
