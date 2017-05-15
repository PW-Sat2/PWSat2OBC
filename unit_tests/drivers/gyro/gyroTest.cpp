#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "I2C/I2CMock.hpp"
#include "OsMock.hpp"
#include "base/reader.h"
#include "base/writer.h"
#include "gyro/driver.hpp"
#include "gyro/telemetry.hpp"
#include "i2c/i2c.h"
#include "os/os.hpp"
#include "system.h"
#include "utils.hpp"

using testing::_;
using testing::Return;
using testing::Invoke;
using testing::ElementsAre;
using testing::InSequence;
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

TEST_F(GyroTest, init_happyCase)
{
    InSequence dummy;

    // hardware reset
    EXPECT_CALL(i2c, Write(_addr, ElementsAre(0x3E, (1 << 7)))).WillOnce(Return(I2CResult::OK));

    // who am I?
    EXPECT_CALL(i2c, WriteRead(_addr, ElementsAre(0), _)).WillOnce(Invoke([](uint8_t, auto, auto read) {
        EXPECT_EQ(read.size(), 1);

        read[0] = 0b1101000;
        return I2CResult::OK;
    }));

    // power management register
    EXPECT_CALL(i2c, Write(_addr, ElementsAre(0x3E, 1))).WillOnce(Return(I2CResult::OK));

    // config
    EXPECT_CALL(i2c, Write(_addr, ElementsAre(0x15, 1, 0b11110, 0b1000101))).WillOnce(Return(I2CResult::OK));

    EXPECT_CALL(os, Sleep(100ms)).WillOnce(Return());

    // check if locked
    EXPECT_CALL(i2c, WriteRead(_addr, ElementsAre(0x1A), _)).WillOnce(Invoke([](uint8_t, auto, auto read) {
        EXPECT_EQ(read.size(), 1);

        read[0] = 0b101;
        return I2CResult::OK;
    }));

    EXPECT_TRUE(gyro.init());
}

TEST_F(GyroTest, init_hardwareResetFailed)
{
    // hardware reset
    EXPECT_CALL(i2c, Write(_addr, ElementsAre(0x3E, (1 << 7)))).WillOnce(Return(I2CResult::Failure));

    EXPECT_FALSE(gyro.init());
}

TEST_F(GyroTest, init_deviceNotPresent)
{
    InSequence dummy;

    // hardware reset
    EXPECT_CALL(i2c, Write(_addr, ElementsAre(0x3E, (1 << 7)))).WillOnce(Return(I2CResult::OK));

    // who am I?
    EXPECT_CALL(i2c, WriteRead(_addr, ElementsAre(0), _)).WillOnce(Invoke([](uint8_t, auto, auto read) {
        EXPECT_EQ(read.size(), 1);

        read[0] = 0;
        return I2CResult::OK;
    }));

    EXPECT_FALSE(gyro.init());
}

TEST_F(GyroTest, init_powerManagementConfigurationFailed)
{
    InSequence dummy;

    // hardware reset
    EXPECT_CALL(i2c, Write(_addr, ElementsAre(0x3E, (1 << 7)))).WillOnce(Return(I2CResult::OK));

    // who am I?
    EXPECT_CALL(i2c, WriteRead(_addr, ElementsAre(0), _)).WillOnce(Invoke([](uint8_t, auto, auto read) {
        EXPECT_EQ(read.size(), 1);

        read[0] = 0b1101000;
        return I2CResult::OK;
    }));

    // power management register
    EXPECT_CALL(i2c, Write(_addr, ElementsAre(0x3E, 1))).WillOnce(Return(I2CResult::Failure));

    EXPECT_FALSE(gyro.init());
}

TEST_F(GyroTest, init_deviceConfigurationFailed)
{
    InSequence dummy;

    // hardware reset
    EXPECT_CALL(i2c, Write(_addr, ElementsAre(0x3E, (1 << 7)))).WillOnce(Return(I2CResult::OK));

    // who am I?
    EXPECT_CALL(i2c, WriteRead(_addr, ElementsAre(0), _)).WillOnce(Invoke([](uint8_t, auto, auto read) {
        EXPECT_EQ(read.size(), 1);

        read[0] = 0b1101000;
        return I2CResult::OK;
    }));

    // power management register
    EXPECT_CALL(i2c, Write(_addr, ElementsAre(0x3E, 1))).WillOnce(Return(I2CResult::OK));

    // config
    EXPECT_CALL(i2c, Write(_addr, ElementsAre(0x15, 1, 0b11110, 0b1000101))).WillOnce(Return(I2CResult::Failure));

    EXPECT_FALSE(gyro.init());
}

TEST_F(GyroTest, init_i2cFailedAfterConfiguration)
{
    InSequence dummy;

    // hardware reset
    EXPECT_CALL(i2c, Write(_addr, ElementsAre(0x3E, (1 << 7)))).WillOnce(Return(I2CResult::OK));

    // who am I?
    EXPECT_CALL(i2c, WriteRead(_addr, ElementsAre(0), _)).WillOnce(Invoke([](uint8_t, auto, auto read) {
        EXPECT_EQ(read.size(), 1);

        read[0] = 0b1101000;
        return I2CResult::OK;
    }));

    // power management register
    EXPECT_CALL(i2c, Write(_addr, ElementsAre(0x3E, 1))).WillOnce(Return(I2CResult::OK));

    // config
    EXPECT_CALL(i2c, Write(_addr, ElementsAre(0x15, 1, 0b11110, 0b1000101))).WillOnce(Return(I2CResult::OK));

    EXPECT_CALL(os, Sleep(100ms)).WillOnce(Return());

    // check if locked
    EXPECT_CALL(i2c, WriteRead(_addr, ElementsAre(0x1A), _)).WillOnce(Invoke([](uint8_t, auto, auto read) {
        EXPECT_EQ(read.size(), 1);

        read[0] = 0b101;
        return I2CResult::Failure;
    }));

    EXPECT_TRUE(gyro.init());
}

TEST_F(GyroTest, init_pllNotLocked)
{
    InSequence dummy;

    // hardware reset
    EXPECT_CALL(i2c, Write(_addr, ElementsAre(0x3E, (1 << 7)))).WillOnce(Return(I2CResult::OK));

    // who am I?
    EXPECT_CALL(i2c, WriteRead(_addr, ElementsAre(0), _)).WillOnce(Invoke([](uint8_t, auto, auto read) {
        EXPECT_EQ(read.size(), 1);

        read[0] = 0b1101000;
        return I2CResult::OK;
    }));

    // power management register
    EXPECT_CALL(i2c, Write(_addr, ElementsAre(0x3E, 1))).WillOnce(Return(I2CResult::OK));

    // config
    EXPECT_CALL(i2c, Write(_addr, ElementsAre(0x15, 1, 0b11110, 0b1000101))).WillOnce(Return(I2CResult::OK));

    EXPECT_CALL(os, Sleep(100ms)).WillOnce(Return());

    // check if locked
    EXPECT_CALL(i2c, WriteRead(_addr, ElementsAre(0x1A), _)).WillOnce(Invoke([](uint8_t, auto, auto read) {
        EXPECT_EQ(read.size(), 1);

        read[0] = 0;
        return I2CResult::OK;
    }));

    EXPECT_FALSE(gyro.init());
}

// -------------------------- read ----------------------------------

TEST_F(GyroTest, read_happyCase)
{
    EXPECT_CALL(i2c, WriteRead(_addr, ElementsAre(0x1A), _)).WillOnce(Invoke([&](uint8_t, auto, auto read) {
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
    EXPECT_TRUE(gyroData.HasValue);

    EXPECT_EQ((1 << 8) | 2, gyroData.Value.Temperature());
    EXPECT_EQ((3 << 8) | 4, gyroData.Value.X());
    EXPECT_EQ((5 << 8) | 6, gyroData.Value.Y());
    EXPECT_EQ((7 << 8) | 8, gyroData.Value.Z());
}

TEST_F(GyroTest, read_I2CFailed)
{
    for (int i2cresult = -9; i2cresult < 0; i2cresult++)
    {
        EXPECT_CALL(i2c, WriteRead(_addr, ElementsAre(0x1A), _)).WillOnce(Invoke([=](uint8_t, auto, auto read) {
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

        EXPECT_FALSE(gyro.read().HasValue);
    }
}

TEST_F(GyroTest, read_dataNotReady)
{
    for (int status = 0; status <= 0xFF; ++status)
    {
        EXPECT_CALL(i2c, WriteRead(_addr, ElementsAre(0x1A), _)).WillOnce(Invoke([=](uint8_t, auto, auto read) {
            EXPECT_EQ(read.size(), 9);

            read[0] = status;

            return I2CResult::OK;
        }));

        if (status == 1)
            EXPECT_TRUE(gyro.read().HasValue);
        else
            EXPECT_FALSE(gyro.read().HasValue);
    }
}
