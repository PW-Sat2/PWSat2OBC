#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gmock/gmock-matchers.h"
#include "base/writer.h"
#include "I2C/I2CMock.hpp"
#include "i2c/i2c.h"
#include "rapidcheck.hpp"
#include "rapidcheck/gtest.h"

#include "gyro/gyro.h"

using testing::_;
using testing::ElementsAre;
using testing::Invoke;
using drivers::i2c::I2CResult;
using namespace devices::gyro;

static const uint8_t _addr = 0x68;

class GyroTest : public testing::Test
{
  public:
    GyroTest() : gyro{i2c} {}

  protected:
    devices::gyro::GyroDriver gyro;
    I2CBusMock i2c;
};

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
    EXPECT_TRUE(gyroData.HasValue);

    EXPECT_EQ(rTemp, gyroData.Value.temperature);
    EXPECT_EQ(rGyroX, gyroData.Value.X);
    EXPECT_EQ(rGyroY, gyroData.Value.Y);
    EXPECT_EQ(rGyroZ, gyroData.Value.Z);
}
