#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gmock/gmock-matchers.h"
#include "OsMock.hpp"
#include "os/os.hpp"

#include "time/timer.h"

using testing::Eq;
using testing::Ne;
using testing::_;
using testing::Return;

bool operator==(const TimePoint& left, const TimePoint& right)
{
    return TimePointEqual(left, right);
}

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

    TimeProvider provider;
    TimeNotificationHandler timeHandler;
    testing::NiceMock<OSMock> os;
    OSReset guard;
};

void TimerTest::Initialize()
{
    this->guard = InstallProxy(&os);
    EXPECT_CALL(os, CreateBinarySemaphore()).WillOnce(Return(reinterpret_cast<void*>(1))).WillOnce(Return(reinterpret_cast<void*>(2)));
    ON_CALL(os, TakeSemaphore(_, _)).WillByDefault(Return(OSResultSuccess));
    ON_CALL(os, GiveSemaphore(_)).WillByDefault(Return(OSResultSuccess));
    EXPECT_TRUE(TimeInitialize(&provider, TimePassedProxy, &timeHandler, nullptr));
}

TEST_F(TimerTest, TestDefaultState)
{
    this->guard = InstallProxy(&os);
    ON_CALL(os, CreateBinarySemaphore()).WillByDefault(Return(reinterpret_cast<void*>(this)));
    const auto result = TimeInitialize(&provider, TimePassedProxy, &timeHandler, nullptr);
    ASSERT_THAT(result, Eq(true));
    ASSERT_THAT(TimeGetCurrentTime(&provider), Eq(0u));
    const auto time = TimeGetCurrentMissionTime(&provider);
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
    TimeAdvanceTime(&provider, 1);
}

TEST_F(TimerTest, TestAdvanceTimeWithNotification)
{
    EXPECT_CALL(timeHandler, OnTimePassed(_)).Times(1);
    Initialize();
    TimeAdvanceTime(&provider, 10001);
}

TEST_F(TimerTest, TestAdvanceTimeWithNotificationCheckNotificationSemaphores)
{
    Initialize();
    EXPECT_CALL(os, TakeSemaphore(provider.timerLock, _)).WillOnce(Return(OSResultSuccess));
    EXPECT_CALL(os, GiveSemaphore(provider.timerLock)).Times(1);
    EXPECT_CALL(os, TakeSemaphore(provider.notificationLock, _)).WillOnce(Return(OSResultSuccess));
    EXPECT_CALL(os, GiveSemaphore(provider.notificationLock)).Times(1);
    TimeAdvanceTime(&provider, 10001);
}

TEST_F(TimerTest, TestAdvanceTimeWithNotificationLockFailure)
{
    EXPECT_CALL(timeHandler, OnTimePassed(_)).Times(0);
    Initialize();
    EXPECT_CALL(os, TakeSemaphore(provider.timerLock, _)).WillOnce(Return(OSResultSuccess));
    EXPECT_CALL(os, GiveSemaphore(provider.timerLock)).Times(1);
    EXPECT_CALL(os, TakeSemaphore(provider.notificationLock, _)).WillOnce(Return(OSResultInvalidOperation));
    EXPECT_CALL(os, GiveSemaphore(provider.notificationLock)).Times(0);
    TimeAdvanceTime(&provider, 10001);
    ASSERT_THAT(TimeGetCurrentTime(&provider), Eq(10001u));
}

TEST_F(TimerTest, TestAdvanceTimeWithTimerLockFailure)
{
    Initialize();
    EXPECT_CALL(os, TakeSemaphore(provider.timerLock, _)).WillOnce(Return(OSResultInvalidOperation));
    EXPECT_CALL(os, GiveSemaphore(provider.timerLock)).Times(0);
    TimeAdvanceTime(&provider, 500);
    ASSERT_THAT(TimeGetCurrentTime(&provider), Eq(0u));
}

TEST_F(TimerTest, TestCurrentTimeAfterJump)
{
    Initialize();
    TimeAdvanceTime(&provider, 10001);
    ASSERT_THAT(TimeGetCurrentTime(&provider), Eq(10001u));
}

TEST_F(TimerTest, TestMultipleJumps)
{
    EXPECT_CALL(timeHandler, OnTimePassed(_)).Times(1);
    Initialize();
    TimeAdvanceTime(&provider, 1000);
    TimeAdvanceTime(&provider, 4500);
    TimeAdvanceTime(&provider, 6000);
    ASSERT_THAT(TimeGetCurrentTime(&provider), Eq(11500u));
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
    ASSERT_THAT(TimeGetCurrentTime(&provider), Eq(446582001u));
}

TEST_F(TimerTest, TestSetTimeLockError)
{
    Initialize();
    EXPECT_CALL(os, TakeSemaphore(provider.timerLock, _)).WillOnce(Return(OSResultInvalidOperation));
    EXPECT_CALL(os, GiveSemaphore(provider.timerLock)).Times(0);
    TimePoint point;
    point.milisecond = 1;
    point.second = 2;
    point.minute = 3;
    point.hour = 4;
    point.day = 5;
    const auto status = TimeSetCurrentTime(&provider, point);
    ASSERT_THAT(status, Eq(false));
    ASSERT_THAT(TimeGetCurrentTime(&provider), Ne(446582001u));
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
    EXPECT_CALL(timeHandler, OnTimePassed(point)).Times(1);
    TimeSetCurrentTime(&provider, point);
}

TEST_F(TimerTest, TestGetCurrentMissionTime)
{
    Initialize();
    TimeAdvanceTime(&provider, 446582001u);
    const auto point = TimeGetCurrentMissionTime(&provider);
    ASSERT_THAT(point.milisecond, Eq(1));
    ASSERT_THAT(point.second, Eq(2));
    ASSERT_THAT(point.minute, Eq(3));
    ASSERT_THAT(point.hour, Eq(4));
    ASSERT_THAT(point.day, Eq(5));
}

TEST_F(TimerTest, TestTimerCallback)
{
    Initialize();
    EXPECT_CALL(timeHandler, OnTimePassed(_)).Times(1);
    auto callback = TimeGetTickProcedure();
    callback(&provider, 446582001u);
    const auto point = TimeGetCurrentMissionTime(&provider);
    ASSERT_THAT(point.milisecond, Eq(1));
    ASSERT_THAT(point.second, Eq(2));
    ASSERT_THAT(point.minute, Eq(3));
    ASSERT_THAT(point.hour, Eq(4));
    ASSERT_THAT(point.day, Eq(5));
}
