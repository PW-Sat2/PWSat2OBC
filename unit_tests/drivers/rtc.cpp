#include "gtest/gtest.h"
#include "I2C/I2CMock.hpp"

#include "rtc/rtc.hpp"

using namespace devices::rtc;

using testing::Return;
using testing::_;
using testing::Eq;
using testing::Ge;
using testing::Ne;
using testing::Invoke;
using testing::ElementsAre;
using gsl::span;
using drivers::i2c::I2CResult;

class RTCObjectTest : public testing::Test
{
  protected:
    RTCObjectTest();
    I2CBusMock i2c;
    RTCObject rtc;
};

RTCObjectTest::RTCObjectTest() : rtc(i2c)
{
}

TEST_F(RTCObjectTest, AllZeros)
{
    EXPECT_CALL(i2c, WriteRead(RTCObject::I2CAddress, ElementsAre((std::uint8_t)Registers::VL_seconds), _))
        .WillOnce(Invoke([=](uint8_t /*address*/, span<const uint8_t> /*inData*/, span<uint8_t> outData) {
            std::fill(outData.begin(), outData.end(), 0);
            return I2CResult::OK;
        }));

    RTCTime time;
    ASSERT_THAT(rtc.ReadTime(time), Eq(OSResult::Success));

    ASSERT_THAT(time.seconds, Eq(0));
    ASSERT_THAT(time.minutes, Eq(0));
    ASSERT_THAT(time.hours, Eq(0));
    ASSERT_THAT(time.days, Eq(0));
    ASSERT_THAT(time.months, Eq(0));
    ASSERT_THAT(time.years, Eq(0));
}

TEST_F(RTCObjectTest, Seconds)
{
    EXPECT_CALL(i2c, WriteRead(RTCObject::I2CAddress, ElementsAre((std::uint8_t)Registers::VL_seconds), _))
        .WillOnce(Invoke([=](uint8_t /*address*/, span<const uint8_t> /*inData*/, span<uint8_t> outData) {
            std::fill(outData.begin(), outData.end(), 0);
            outData[0] = 0x23;
            return I2CResult::OK;
        }));

    RTCTime time;
    ASSERT_THAT(rtc.ReadTime(time), Eq(OSResult::Success));

    ASSERT_THAT(time.seconds, Eq(23));
    ASSERT_THAT(time.minutes, Eq(0));
    ASSERT_THAT(time.hours, Eq(0));
    ASSERT_THAT(time.days, Eq(0));
    ASSERT_THAT(time.months, Eq(0));
    ASSERT_THAT(time.years, Eq(0));
}

TEST_F(RTCObjectTest, Minutes)
{
    EXPECT_CALL(i2c, WriteRead(RTCObject::I2CAddress, ElementsAre((std::uint8_t)Registers::VL_seconds), _))
        .WillOnce(Invoke([=](uint8_t /*address*/, span<const uint8_t> /*inData*/, span<uint8_t> outData) {
            std::fill(outData.begin(), outData.end(), 0);
            outData[1] = 0x23;
            return I2CResult::OK;
        }));

    RTCTime time;
    ASSERT_THAT(rtc.ReadTime(time), Eq(OSResult::Success));

    ASSERT_THAT(time.seconds, Eq(0));
    ASSERT_THAT(time.minutes, Eq(23));
    ASSERT_THAT(time.hours, Eq(0));
    ASSERT_THAT(time.days, Eq(0));
    ASSERT_THAT(time.months, Eq(0));
    ASSERT_THAT(time.years, Eq(0));
}

TEST_F(RTCObjectTest, Hours)
{
    EXPECT_CALL(i2c, WriteRead(RTCObject::I2CAddress, ElementsAre((std::uint8_t)Registers::VL_seconds), _))
        .WillOnce(Invoke([=](uint8_t /*address*/, span<const uint8_t> /*inData*/, span<uint8_t> outData) {
            std::fill(outData.begin(), outData.end(), 0);
            outData[2] = 0x23;
            return I2CResult::OK;
        }));

    RTCTime time;
    ASSERT_THAT(rtc.ReadTime(time), Eq(OSResult::Success));

    ASSERT_THAT(time.seconds, Eq(0));
    ASSERT_THAT(time.minutes, Eq(0));
    ASSERT_THAT(time.hours, Eq(23));
    ASSERT_THAT(time.days, Eq(0));
    ASSERT_THAT(time.months, Eq(0));
    ASSERT_THAT(time.years, Eq(0));
}

TEST_F(RTCObjectTest, Days)
{
    EXPECT_CALL(i2c, WriteRead(RTCObject::I2CAddress, ElementsAre((std::uint8_t)Registers::VL_seconds), _))
        .WillOnce(Invoke([=](uint8_t /*address*/, span<const uint8_t> /*inData*/, span<uint8_t> outData) {
            std::fill(outData.begin(), outData.end(), 0);
            outData[3] = 0x23;
            return I2CResult::OK;
        }));

    RTCTime time;
    ASSERT_THAT(rtc.ReadTime(time), Eq(OSResult::Success));

    ASSERT_THAT(time.seconds, Eq(0));
    ASSERT_THAT(time.minutes, Eq(0));
    ASSERT_THAT(time.hours, Eq(0));
    ASSERT_THAT(time.days, Eq(23));
    ASSERT_THAT(time.months, Eq(0));
    ASSERT_THAT(time.years, Eq(0));
}

TEST_F(RTCObjectTest, IgnoredField)
{
    EXPECT_CALL(i2c, WriteRead(RTCObject::I2CAddress, ElementsAre((std::uint8_t)Registers::VL_seconds), _))
        .WillOnce(Invoke([=](uint8_t /*address*/, span<const uint8_t> /*inData*/, span<uint8_t> outData) {
            std::fill(outData.begin(), outData.end(), 0);
            outData[4] = 13;
            return I2CResult::OK;
        }));

    RTCTime time;
    ASSERT_THAT(rtc.ReadTime(time), Eq(OSResult::Success));

    ASSERT_THAT(time.seconds, Eq(0));
    ASSERT_THAT(time.minutes, Eq(0));
    ASSERT_THAT(time.hours, Eq(0));
    ASSERT_THAT(time.days, Eq(0));
    ASSERT_THAT(time.months, Eq(0));
    ASSERT_THAT(time.years, Eq(0));
}

TEST_F(RTCObjectTest, Months)
{
    EXPECT_CALL(i2c, WriteRead(RTCObject::I2CAddress, ElementsAre((std::uint8_t)Registers::VL_seconds), _))
        .WillOnce(Invoke([=](uint8_t /*address*/, span<const uint8_t> /*inData*/, span<uint8_t> outData) {
            std::fill(outData.begin(), outData.end(), 0);
            outData[5] = 0x11;
            return I2CResult::OK;
        }));

    RTCTime time;
    ASSERT_THAT(rtc.ReadTime(time), Eq(OSResult::Success));

    ASSERT_THAT(time.seconds, Eq(0));
    ASSERT_THAT(time.minutes, Eq(0));
    ASSERT_THAT(time.hours, Eq(0));
    ASSERT_THAT(time.days, Eq(0));
    ASSERT_THAT(time.months, Eq(11));
    ASSERT_THAT(time.years, Eq(0));
}

TEST_F(RTCObjectTest, Years)
{
    EXPECT_CALL(i2c, WriteRead(RTCObject::I2CAddress, ElementsAre((std::uint8_t)Registers::VL_seconds), _))
        .WillOnce(Invoke([=](uint8_t /*address*/, span<const uint8_t> /*inData*/, span<uint8_t> outData) {
            std::fill(outData.begin(), outData.end(), 0);
            outData[6] = 0x23;
            return I2CResult::OK;
        }));

    RTCTime time;
    ASSERT_THAT(rtc.ReadTime(time), Eq(OSResult::Success));

    ASSERT_THAT(time.seconds, Eq(0));
    ASSERT_THAT(time.minutes, Eq(0));
    ASSERT_THAT(time.hours, Eq(0));
    ASSERT_THAT(time.days, Eq(0));
    ASSERT_THAT(time.months, Eq(0));
    ASSERT_THAT(time.years, Eq(23));
}

TEST_F(RTCObjectTest, FullRTCDate)
{
    EXPECT_CALL(i2c, WriteRead(RTCObject::I2CAddress, ElementsAre((std::uint8_t)Registers::VL_seconds), _))
        .WillOnce(Invoke([=](uint8_t /*address*/, span<const uint8_t> /*inData*/, span<uint8_t> outData) {
            std::fill(outData.begin(), outData.end(), 0);
            // 23, 58, 10, 16, 0, 3, 17
            outData[0] = 0x23;
            outData[1] = 0x58;
            outData[2] = 0x10;
            outData[3] = 0x16;
            outData[4] = 0x00;
            outData[5] = 0x03;
            outData[6] = 0x17;
            return I2CResult::OK;
        }));

    RTCTime time;
    ASSERT_THAT(rtc.ReadTime(time), Eq(OSResult::Success));

    ASSERT_THAT(time.seconds, Eq(23));
    ASSERT_THAT(time.minutes, Eq(58));
    ASSERT_THAT(time.hours, Eq(10));
    ASSERT_THAT(time.days, Eq(16));
    ASSERT_THAT(time.months, Eq(3));
    ASSERT_THAT(time.years, Eq(17));
}

TEST(RTCTimeTest, DecodeRTCTimeProperly)
{
    RTCTime rtcTime;
    memset(&rtcTime, 0, sizeof(rtcTime));
    rtcTime.years = 17;
    rtcTime.months = 3;
    rtcTime.days = 31;
    rtcTime.hours = 5;
    rtcTime.minutes = 10;
    rtcTime.seconds = 59;

    auto sinceEpoch = rtcTime.ToDuration();

    ASSERT_THAT(sinceEpoch.count(), Eq(1490937059));
}
