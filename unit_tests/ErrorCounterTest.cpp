#include <array>
#include <cstdint>
#include <iostream>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "base/os.h"
#include "error_counter/error_counter.hpp"
#include "mock/error_counter.hpp"

using testing::Eq;
using testing::NiceMock;
using testing::Return;
using testing::_;

using namespace error_counter;

constexpr Device DeviceA = 1;
constexpr Device DeviceB = 2;

struct ErrorCountingCallbackMock : public IErrorCountingCallback
{
    MOCK_METHOD2(LimitReached, void(Device, CounterValue));
};

class ErrorCounterTest : public testing::Test
{
  public:
    ErrorCounterTest();

  protected:
    NiceMock<ErrorCountingCallbackMock> _callback;
    NiceMock<ErrorCountingConfigrationMock> _config;

    ErrorCounting _counting;
    ErrorCounter<DeviceA> _deviceA;
    ErrorCounter<DeviceB> _deviceB;
};

ErrorCounterTest::ErrorCounterTest()
    : _counting(this->_config),  //
      _deviceA(this->_counting), //
      _deviceB(this->_counting)
{
    this->_counting.Handler(this->_callback);
}

TEST_F(ErrorCounterTest, ShouldStartWithZero)
{
    auto r = this->_deviceA.Current();
    ASSERT_THAT(r, Eq(0));
}

TEST_F(ErrorCounterTest, ShouldIncrementCounterOnFailure)
{
    ON_CALL(this->_config, Increment(DeviceA)).WillByDefault(Return(2));
    this->_deviceA.Failure();

    ASSERT_THAT(this->_deviceA.Current(), Eq(2));
}

TEST_F(ErrorCounterTest, ShouldClearCounterOnSuccess)
{
    this->_deviceA.Failure();
    this->_deviceA.Failure();

    this->_deviceA.Success();

    ASSERT_THAT(this->_deviceA.Current(), Eq(0));
}

TEST_F(ErrorCounterTest, TwoCountersShouldBeIndependent)
{
    this->_deviceA.Failure();
    this->_deviceA.Failure();
    this->_deviceB.Failure();
    this->_deviceA.Failure();

    ASSERT_THAT(this->_deviceA.Current(), Eq(3));
    ASSERT_THAT(this->_deviceB.Current(), Eq(1));
}

TEST_F(ErrorCounterTest, ShouldTriggerCallbackWhenFailureLimitIsReach)
{
    EXPECT_CALL(_callback, LimitReached(DeviceA, 7));

    ON_CALL(this->_config, Limit(DeviceA)).WillByDefault(Return(7));

    std::cout << (int)this->_deviceA.Current() << std::endl;
    this->_deviceA.Failure();
    std::cout << (int)this->_deviceA.Current() << std::endl;
    this->_deviceA.Failure();
    std::cout << (int)this->_deviceA.Current() << std::endl;
    this->_deviceA.Failure();
    std::cout << (int)this->_deviceA.Current() << std::endl;
    this->_deviceA.Failure();
    std::cout << (int)this->_deviceA.Current() << std::endl;
    this->_deviceA.Failure();
    std::cout << (int)this->_deviceA.Current() << std::endl;
    this->_deviceA.Failure();
    std::cout << (int)this->_deviceA.Current() << std::endl;
    this->_deviceA.Failure();
    std::cout << (int)this->_deviceA.Current() << std::endl;
}

TEST_F(ErrorCounterTest, ShouldUpdateCounterBasedOnFlag)
{
    false >> this->_deviceA;
    false >> this->_deviceA;

    ASSERT_THAT(this->_deviceA, Eq(2));

    true >> this->_deviceA;

    ASSERT_THAT(this->_deviceA, Eq(0));
}
