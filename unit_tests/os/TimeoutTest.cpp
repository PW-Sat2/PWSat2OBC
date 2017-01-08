#include <cstdint>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "OsMock.hpp"
#include "base/os.h"
#include "os.hpp"

using testing::Test;
using testing::Return;
using testing::Eq;

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
    EXPECT_CALL(this->_os, GetUptime()).WillRepeatedly(Return(OSTaskTimeSpan(0)));

    Timeout t(OSTaskTimeSpan(0));

    ASSERT_THAT(t.Expired(), Eq(true));
}

TEST_F(TimeoutTest, TimeoutWillExpireAfterSpecifiedNumberOfMiliseconds)
{
    EXPECT_CALL(this->_os, GetUptime()).WillRepeatedly(Return(OSTaskTimeSpan(100)));

    Timeout t(OSTaskTimeSpan(10));

    ASSERT_THAT(t.Expired(), Eq(false));

    EXPECT_CALL(this->_os, GetUptime()).WillRepeatedly(Return(OSTaskTimeSpan(110)));

    ASSERT_THAT(t.Expired(), Eq(true));
}
