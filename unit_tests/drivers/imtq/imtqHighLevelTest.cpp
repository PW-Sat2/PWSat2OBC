#include <algorithm>
#include <em_i2c.h>
#include <gsl/span>
#include <string>
#include <tuple>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gmock/gmock-matchers.h"
#include "I2C/I2CMock.hpp"
#include "OsMock.hpp"
#include "base/reader.h"
#include "base/writer.h"
#include "i2c/i2c.h"
#include "os/os.hpp"
#include "system.h"
#include "utils.hpp"

#include "imtq/imtq.h"
#include "system.h"

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
using namespace std::chrono_literals;

using namespace devices::imtq;
namespace
{
    static const uint8_t ImtqAddress = 0x10;

#define FOR_AXIS(var) for (uint8_t var = 0; var < 3; ++var)

    class ImtqUseTest : public testing::Test
    {
      public:
        ImtqUseTest() : imtq{i2c}
        {
            this->_reset = InstallProxy(&os);
        }

      protected:
        ImtqDriver imtq;
        OSMock os;
        OSReset _reset;
        I2CBusMock i2c;
    };

    TEST_F(ImtqUseTest, PerformSelfTest)
    {
        EXPECT_CALL(i2c, Write(ImtqAddress, ElementsAre(0x08, 0x00))).WillOnce(Return(I2CResult::OK));
        EXPECT_CALL(i2c, Write(ImtqAddress, ElementsAre(0x47))).WillOnce(Return(I2CResult::OK));

        EXPECT_CALL(i2c, Read(ImtqAddress, _))
            .WillOnce(Invoke([=](uint8_t /*address*/, auto outData) {
                EXPECT_EQ(outData.size(), 2);
                outData[0] = 0x08;
                outData[1] = 0x00;
                return I2CResult::OK;
            }))
            .WillOnce(Invoke([=](uint8_t /*address*/, auto outData) {
                EXPECT_EQ(outData.size(), 320);

                outData[0] = 0x47;
                for (int i = 1; i < 320; ++i)
                {
                    outData[i] = 0x00;
                }
                return I2CResult::OK;
            }));

        EXPECT_CALL(os, Sleep(500ms)).WillOnce(Return());
        EXPECT_CALL(os, Sleep(10ms)).WillRepeatedly(Return());

        SelfTestResult result;
        EXPECT_TRUE(imtq.PerformSelfTest(result));

        for (int i = 0; i < 8; ++i)
        {
            EXPECT_EQ(result.stepResults[i].actualStep, SelfTestResult::Step::Init);
            EXPECT_EQ(result.stepResults[i].error.GetValue(), 0);
            for (int j = 0; j < 3; ++j)
            {
                EXPECT_EQ(result.stepResults[i].CalibratedMagnetometerMeasurement[j], 0);
                EXPECT_EQ(result.stepResults[i].CoilCurrent[j], 0);
                EXPECT_EQ(result.stepResults[i].CoilTemperature[j], 0);
                EXPECT_EQ(result.stepResults[i].RawMagnetometerMeasurement[j], 0);
            }
        }
    }

    TEST_F(ImtqUseTest, MeasureMagnetometer)
    {
        EXPECT_CALL(i2c, Write(ImtqAddress, ElementsAre(0x03))).WillOnce(Return(I2CResult::OK));
        EXPECT_CALL(i2c, Write(ImtqAddress, ElementsAre(0x04))).WillOnce(Return(I2CResult::OK));
        EXPECT_CALL(i2c, Write(ImtqAddress, ElementsAre(0x43))).WillOnce(Return(I2CResult::OK));

        EXPECT_CALL(i2c, Read(ImtqAddress, _))
            .WillOnce(Invoke([=](uint8_t /*address*/, auto outData) {
                EXPECT_EQ(outData.size(), 2);
                outData[0] = 0x03;
                outData[1] = 0x00;
                return I2CResult::OK;
            }))
            .WillOnce(Invoke([=](uint8_t /*address*/, auto outData) {
                EXPECT_EQ(outData.size(), 2);
                outData[0] = 0x04;
                outData[1] = 0x00;
                return I2CResult::OK;
            }))
            .WillOnce(Invoke([=](uint8_t /*address*/, auto outData) {
                EXPECT_EQ(outData.size(), 15);

                outData[0] = 0x43;
                for (int i = 1; i < 15; ++i)
                {
                    outData[i] = 0x00;
                }
                return I2CResult::OK;
            }));

        EXPECT_CALL(os, Sleep(30ms)).WillOnce(Return());
        EXPECT_CALL(os, Sleep(10ms)).WillRepeatedly(Return());

        Vector3<MagnetometerMeasurement> value = {1, 2, 3};
        EXPECT_TRUE(imtq.MeasureMagnetometer(value));

        for (int i = 0; i < 3; ++i)
        {
            EXPECT_EQ(value[i], 0);
        }
    }
}
