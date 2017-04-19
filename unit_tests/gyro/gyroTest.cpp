#include <algorithm>
#include <em_i2c.h>
#include <gsl/span>
#include <string>
#include <tuple>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gmock/gmock-matchers.h"
#include "OsMock.hpp"
#include "base/reader.h"
#include "base/writer.h"
#include "I2C/I2CMock.hpp"
#include "i2c/i2c.h"
#include "os/os.hpp"
#include "rapidcheck.hpp"
#include "rapidcheck/gtest.h"
#include "system.h"
#include "utils.hpp"

#include "gyro/gyro.h"

using testing::_;
using testing::Eq;
using testing::Ne;
using testing::Ge;
using testing::StrEq;
using testing::Return;
using testing::Invoke;
using testing::Pointee;
using testing::ElementsAre;
using testing::Matches;
using gsl::span;
using drivers::i2c::I2CResult;
using namespace devices::gyro;
using namespace std::chrono_literals;

static const uint8_t _addr = 0x68;

class GyroTest : public testing::Test
{
  public:
    GyroTest() : gyro(i2c)
    {
        this->_reset = InstallProxy(&os);
    }

  protected:
    devices::gyro::GyroDriver gyro;
    I2CBusMock i2c;
    OSMock os;
    OSReset _reset;
};

// -------------------------- init ----------------------------------

TEST_F(GyroTest, initHappyCase)
{
    // hardware reset
    EXPECT_CALL(i2c, Write(_addr, ElementsAre(0x3E, (1 << 7)))).
            WillOnce(Return(I2CResult::OK));

    // who am I?
    EXPECT_CALL(i2c, WriteRead(_addr, ElementsAre(0), _)).
            WillOnce(Invoke([](uint8_t, auto, auto read) {
        EXPECT_EQ(read.size(), 1);

        read[0] = 0b1101000;
        return I2CResult::OK;
    }));

    // power management register
    EXPECT_CALL(i2c, Write(_addr, ElementsAre(0x3E, 1))).
            WillOnce(Return(I2CResult::OK));

    // config
    EXPECT_CALL(i2c, Write(_addr, ElementsAre(0x15, 1, 0b11110, 0b1000101))).
            WillOnce(Return(I2CResult::OK));

    EXPECT_CALL(os, Sleep(100ms)).WillOnce(Return());

    // check if locked
    EXPECT_CALL(i2c, WriteRead(_addr, ElementsAre(0x1A), _)).
            WillOnce(Invoke([](uint8_t, auto, auto read) {
        EXPECT_EQ(read.size(), 1);

        read[0] = 0b101;
        return I2CResult::OK;
    }));

    EXPECT_TRUE(gyro.init());
}

TEST_F(GyroTest, hardwareResetFailed)
{
    // hardware reset
    EXPECT_CALL(i2c, Write(_addr, ElementsAre(0x3E, (1 << 7)))).
            WillOnce(Return(I2CResult::Failure));

    EXPECT_FALSE(gyro.init());
}

TEST_F(GyroTest, deviceNotPresent)
{
    // hardware reset
    EXPECT_CALL(i2c, Write(_addr, ElementsAre(0x3E, (1 << 7)))).
            WillOnce(Return(I2CResult::OK));

    // who am I?
    EXPECT_CALL(i2c, WriteRead(_addr, ElementsAre(0), _)).
            WillOnce(Invoke([](uint8_t, auto, auto read) {
        EXPECT_EQ(read.size(), 1);

        read[0] = 0;
        return I2CResult::OK;
    }));

    EXPECT_FALSE(gyro.init());
}

TEST_F(GyroTest, config1Failed)
{
    // hardware reset
        EXPECT_CALL(i2c, Write(_addr, ElementsAre(0x3E, (1 << 7)))).
                WillOnce(Return(I2CResult::OK));

    // who am I?
    EXPECT_CALL(i2c, WriteRead(_addr, ElementsAre(0), _)).
            WillOnce(Invoke([](uint8_t, auto, auto read) {
        EXPECT_EQ(read.size(), 1);

        read[0] = 0b1101000;
        return I2CResult::OK;
    }));

    // power management register
    EXPECT_CALL(i2c, Write(_addr, ElementsAre(0x3E, 1))).
            WillOnce(Return(I2CResult::Failure));

    EXPECT_FALSE(gyro.init());
}

TEST_F(GyroTest, config2Failed)
{
    // hardware reset
    EXPECT_CALL(i2c, Write(_addr, ElementsAre(0x3E, (1 << 7)))).
            WillOnce(Return(I2CResult::OK));

    // who am I?
    EXPECT_CALL(i2c, WriteRead(_addr, ElementsAre(0), _)).
            WillOnce(Invoke([](uint8_t, auto, auto read) {
        EXPECT_EQ(read.size(), 1);

        read[0] = 0b1101000;
        return I2CResult::OK;
    }));

    // power management register
    EXPECT_CALL(i2c, Write(_addr, ElementsAre(0x3E, 1))).
            WillOnce(Return(I2CResult::OK));

    // config
    EXPECT_CALL(i2c, Write(_addr, ElementsAre(0x15, 1, 0b11110, 0b1000101))).
            WillOnce(Return(I2CResult::Failure));

    EXPECT_FALSE(gyro.init());
}

TEST_F(GyroTest, i2cFailedAfterConfiguration)
{
    // hardware reset
    EXPECT_CALL(i2c, Write(_addr, ElementsAre(0x3E, (1 << 7)))).
            WillOnce(Return(I2CResult::OK));

    // who am I?
    EXPECT_CALL(i2c, WriteRead(_addr, ElementsAre(0), _)).
            WillOnce(Invoke([](uint8_t, auto, auto read) {
        EXPECT_EQ(read.size(), 1);

        read[0] = 0b1101000;
        return I2CResult::OK;
    }));

    // power management register
    EXPECT_CALL(i2c, Write(_addr, ElementsAre(0x3E, 1))).
            WillOnce(Return(I2CResult::OK));

    // config
    EXPECT_CALL(i2c, Write(_addr, ElementsAre(0x15, 1, 0b11110, 0b1000101))).
            WillOnce(Return(I2CResult::OK));

    EXPECT_CALL(os, Sleep(100ms)).WillOnce(Return());

    // check if locked
    EXPECT_CALL(i2c, WriteRead(_addr, ElementsAre(0x1A), _)).
            WillOnce(Invoke([](uint8_t, auto, auto read) {
        EXPECT_EQ(read.size(), 1);

        read[0] = 0b101;
        return I2CResult::Failure;
    }));

    EXPECT_TRUE(gyro.init());
}

TEST_F(GyroTest, pllNotLocked)
{
    // hardware reset
    EXPECT_CALL(i2c, Write(_addr, ElementsAre(0x3E, (1 << 7)))).
            WillOnce(Return(I2CResult::OK));

    // who am I?
    EXPECT_CALL(i2c, WriteRead(_addr, ElementsAre(0), _)).
            WillOnce(Invoke([](uint8_t, auto, auto read) {
        EXPECT_EQ(read.size(), 1);

        read[0] = 0b1101000;
        return I2CResult::OK;
    }));

    // power management register
    EXPECT_CALL(i2c, Write(_addr, ElementsAre(0x3E, 1))).
            WillOnce(Return(I2CResult::OK));

    // config
    EXPECT_CALL(i2c, Write(_addr, ElementsAre(0x15, 1, 0b11110, 0b1000101))).
            WillOnce(Return(I2CResult::OK));

    EXPECT_CALL(os, Sleep(100ms)).WillOnce(Return());

    // check if locked
    EXPECT_CALL(i2c, WriteRead(_addr, ElementsAre(0x1A), _)).
            WillOnce(Invoke([](uint8_t, auto, auto read) {
        EXPECT_EQ(read.size(), 1);

        read[0] = 0;
        return I2CResult::OK;
    }));

    EXPECT_FALSE(gyro.init());
}

// -------------------------- read ----------------------------------

TEST_F(GyroTest, readHappyCase)
{
    EXPECT_CALL(i2c, WriteRead(_addr, ElementsAre(0x1A), _)).
            WillOnce(Invoke([&](uint8_t, auto, auto read) {
        EXPECT_EQ(read.size(), 9);

        read[0] = 0b1;
        read[1] = 1;
        read[2] = 2;
        read[3] = 3;
        read[4] = 4;
        read[5] = 5;
        read[6] = 6;
        read[7] = 7;
        read[8] = 8;
        return I2CResult::OK;
    }));

    auto gyroData = gyro.read();
    EXPECT_TRUE(gyroData);

    EXPECT_EQ((1 << 8) | 2, gyroData->temperature);
    EXPECT_EQ((3 << 8) | 4, gyroData->X);
    EXPECT_EQ((5 << 8) | 6, gyroData->Y);
    EXPECT_EQ((7 << 8) | 8, gyroData->Z);
}

RC_GTEST_FIXTURE_PROP(GyroTest, rcHappyCase, (int16_t rTemp, int16_t rGyroX, int16_t rGyroY, int16_t rGyroZ))
{
    EXPECT_CALL(i2c, WriteRead(_addr, ElementsAre(0x1A), _)).
            WillOnce(Invoke([&](uint8_t, auto, auto read) {
        EXPECT_EQ(read.size(), 9);

        Writer writer{read};
        writer.WriteByte(0b1);

        writer.WriteLowerBytesBE(rTemp, 2);
        writer.WriteLowerBytesBE(rGyroX, 2);
        writer.WriteLowerBytesBE(rGyroY, 2);
        writer.WriteLowerBytesBE(rGyroZ, 2);


        return I2CResult::OK;
    }));

    auto gyroData = gyro.read();
    EXPECT_TRUE(gyroData);

    EXPECT_EQ(rTemp, gyroData->temperature);
    EXPECT_EQ(rGyroX, gyroData->X);
    EXPECT_EQ(rGyroY, gyroData->Y);
    EXPECT_EQ(rGyroZ, gyroData->Z);
}

TEST_F(GyroTest, readI2CFail)
{
    for(int i2cresult = -9; i2cresult < 0; i2cresult++)
    {
        EXPECT_CALL(i2c, WriteRead(_addr, ElementsAre(0x1A), _)).
                WillOnce(Invoke([=](uint8_t, auto, auto read) {
            EXPECT_EQ(read.size(), 9);

            read[0] = 0b1;
            read[1] = 1;
            read[2] = 2;
            read[3] = 3;
            read[4] = 4;
            read[5] = 5;
            read[6] = 6;
            read[7] = 7;
            read[8] = 8;

            return static_cast<I2CResult>(i2cresult);
        }));

        EXPECT_FALSE(gyro.read());
    }
}

TEST_F(GyroTest, readGyroNotReady)
{
    for(int status = 0; status <= 0xFF; ++status)
    {
        EXPECT_CALL(i2c, WriteRead(_addr, ElementsAre(0x1A), _)).
                WillOnce(Invoke([=](uint8_t, auto, auto read) {
            EXPECT_EQ(read.size(), 9);

            read[0] = status;

            return I2CResult::OK;
        }));

        if (status == 1)
            EXPECT_TRUE(gyro.read());
        else
            EXPECT_FALSE(gyro.read());
    }
}
