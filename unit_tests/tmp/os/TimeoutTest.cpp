#include <cstdint>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "OsMock.hpp"
#include "base/os.h"

using testing::Test;
using testing::Return;
using testing::Eq;
using namespace std::chrono_literals;

class TimeoutTest : public Test
{
  public:
    TimeoutTest();

  protected:
    OSMock _os;
    OSReset _reset;
};

TimeoutTest::TimeoutTest()
{
    this->_reset = InstallProxy(&this->_os);
}

TEST_F(TimeoutTest, ZeroTimeoutWillExpireImmediately)
{
    EXPECT_CALL(this->_os, GetUptime()).WillRepeatedly(Return(0ms));

    Timeout t(0ms);

    ASSERT_THAT(t.Expired(), Eq(true));
}

TEST_F(TimeoutTest, TimeoutWillExpireAfterSpecifiedNumberOfMiliseconds)
{
    EXPECT_CALL(this->_os, GetUptime()).WillRepeatedly(Return(100ms));

    Timeout t(10ms);

    ASSERT_THAT(t.Expired(), Eq(false));

    EXPECT_CALL(this->_os, GetUptime()).WillRepeatedly(Return(110ms));

    ASSERT_THAT(t.Expired(), Eq(true));
}
