#include <chrono>

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "OsMock.hpp"
#include "base/os.h"

using testing::NiceMock;
using testing::Return;
using testing::Eq;
using namespace std::chrono_literals;

class EventGroupTest : public testing::Test
{
  public:
    EventGroupTest();

  protected:
    NiceMock<OSMock> _os;
    OSReset _osReset;
};

inline EventGroupTest::EventGroupTest()
{
    this->_osReset = InstallProxy(&this->_os);
}

TEST_F(EventGroupTest, ShouldCreateEventGroup)
{
    EXPECT_CALL(this->_os, CreateEventGroup()).WillOnce(Return(reinterpret_cast<OSEventGroupHandle>(1)));

    EventGroup eg;
    auto r = eg.Initialize();

    ASSERT_THAT(r, Eq(OSResult::Success));
}

TEST_F(EventGroupTest, ShouldCreateEventGroupAndDetectFail)
{
    EXPECT_CALL(this->_os, CreateEventGroup()).WillOnce(Return(reinterpret_cast<OSEventGroupHandle>(0)));

    EventGroup eg;
    auto r = eg.Initialize();

    ASSERT_THAT(r, Eq(OSResult::NotEnoughMemory));
}

TEST_F(EventGroupTest, ShouldSetBits)
{
    OSEventGroupHandle handle = reinterpret_cast<OSEventGroupHandle>(1);
    EXPECT_CALL(this->_os, CreateEventGroup()).WillOnce(Return(handle));
    EXPECT_CALL(this->_os, EventGroupSetBits(handle, 1 << 2)).Times(1);

    EventGroup eg;
    eg.Initialize();

    eg.Set(1 << 2);
}

TEST_F(EventGroupTest, ShouldSetBitsISR)
{
    OSEventGroupHandle handle = reinterpret_cast<OSEventGroupHandle>(1);
    EXPECT_CALL(this->_os, CreateEventGroup()).WillOnce(Return(handle));
    EXPECT_CALL(this->_os, EventGroupSetBitsISR(handle, 1 << 2)).Times(1);

    EventGroup eg;
    eg.Initialize();

    eg.SetISR(1 << 2);
}

TEST_F(EventGroupTest, ShouldClearBits)
{
    OSEventGroupHandle handle = reinterpret_cast<OSEventGroupHandle>(1);
    EXPECT_CALL(this->_os, CreateEventGroup()).WillOnce(Return(handle));
    EXPECT_CALL(this->_os, EventGroupClearBits(handle, 1 << 2)).Times(1);

    EventGroup eg;
    eg.Initialize();

    eg.Clear(1 << 2);
}

TEST_F(EventGroupTest, ShouldWaitForAnyBits)
{
    OSEventGroupHandle handle = reinterpret_cast<OSEventGroupHandle>(1);
    EXPECT_CALL(this->_os, CreateEventGroup()).WillOnce(Return(handle));
    EXPECT_CALL(this->_os, EventGroupWaitForBits(handle, (1 << 2) || (1 << 4), false, true, 2ms)).WillOnce(Return(1 << 2));

    EventGroup eg;
    eg.Initialize();

    auto r = eg.WaitAny((1 << 2) || (1 << 4), true, 2ms);

    ASSERT_THAT(r, Eq(1U << 2));
}

TEST_F(EventGroupTest, ShouldWaitForAllBits)
{
    OSEventGroupHandle handle = reinterpret_cast<OSEventGroupHandle>(1);
    EXPECT_CALL(this->_os, CreateEventGroup()).WillOnce(Return(handle));
    EXPECT_CALL(this->_os, EventGroupWaitForBits(handle, (1 << 2) || (1 << 4), true, true, 2ms)).WillOnce(Return(1 << 2));

    EventGroup eg;
    eg.Initialize();

    auto r = eg.WaitAll((1 << 2) || (1 << 4), true, 2ms);

    ASSERT_THAT(r, Eq(1U << 2));
}
