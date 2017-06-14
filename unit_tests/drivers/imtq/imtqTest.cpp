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
#include "mock/error_counter.hpp"
#include "os/os.hpp"
#include "system.h"
#include "utils.hpp"

#include "imtq/imtq.h"

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
using namespace devices::imtq;
namespace
{
    static const uint8_t ImtqAddress = 0x10;

    class ImtqTest : public testing::Test
    {
      public:
        ImtqTest() : errors{errorsConfig}, imtq{errors, i2c}, error_counter{errors}
        {
        }

      protected:
        testing::NiceMock<ErrorCountingConfigrationMock> errorsConfig;
        error_counter::ErrorCounting errors;
        devices::imtq::ImtqDriver imtq;
        I2CBusMock i2c;
        devices::imtq::ImtqDriver::ErrorCounter error_counter;
    };

    TEST_F(ImtqTest, Accepted)
    {
        // accepted
        EXPECT_CALL(i2c, Write(ImtqAddress, ElementsAre(0x02))).WillOnce(Return(I2CResult::OK));
        EXPECT_CALL(i2c, Read(ImtqAddress, _)).WillOnce(Invoke([](uint8_t /*address*/, auto outData) {
            EXPECT_EQ(outData.size(), 2);
            outData[0] = 0x02;
            outData[1] = 0;
            return I2CResult::OK;
        }));

        auto status = imtq.SendNoOperation();
        EXPECT_TRUE(status);
        EXPECT_EQ(imtq.LastError, ImtqDriverError::OK);
        EXPECT_EQ(error_counter, 0);
    }

    TEST_F(ImtqTest, StatusError)
    {
        for (int response = 1; response < 0b10000000; ++response)
        {
            EXPECT_CALL(i2c, Write(ImtqAddress, ElementsAre(0x02))).WillOnce(Return(I2CResult::OK));
            EXPECT_CALL(i2c, Read(ImtqAddress, _)).WillOnce(Invoke([=](uint8_t /*address*/, auto outData) {
                EXPECT_EQ(outData.size(), 2);
                outData[0] = 0x02;
                outData[1] = response;
                return I2CResult::OK;
            }));
            auto status = imtq.SendNoOperation();
            EXPECT_FALSE(status);
            EXPECT_EQ(imtq.LastError, ImtqDriverError::StatusError);
            EXPECT_EQ(imtq.LastStatus, response);
            EXPECT_EQ(error_counter, 0);
        }
    }

    TEST_F(ImtqTest, BadOpcodeResponse)
    {
        EXPECT_CALL(i2c, Write(ImtqAddress, ElementsAre(0x02))).WillOnce(Return(I2CResult::OK));
        EXPECT_CALL(i2c, Read(ImtqAddress, _)).WillOnce(Invoke([](uint8_t /*address*/, auto outData) {
            EXPECT_EQ(outData.size(), 2);
            outData[0] = 0x01;
            outData[1] = 0;
            return I2CResult::OK;
        }));
        auto status = imtq.SendNoOperation();
        EXPECT_FALSE(status);
        EXPECT_EQ(imtq.LastError, ImtqDriverError::WrongOpcodeInResponse);
        EXPECT_EQ(error_counter, 0);
    }

    TEST_F(ImtqTest, I2CWriteFail)
    {
        EXPECT_CALL(i2c, Write(ImtqAddress, ElementsAre(0x02))).WillOnce(Return(I2CResult::Failure));
        auto status = imtq.SendNoOperation();
        EXPECT_FALSE(status);
        EXPECT_EQ(imtq.LastError, ImtqDriverError::I2CWriteFailed);
        EXPECT_EQ(error_counter, 5);
    }

    TEST_F(ImtqTest, I2CReadFail)
    {
        EXPECT_CALL(i2c, Write(ImtqAddress, ElementsAre(0x02))).WillOnce(Return(I2CResult::OK));
        EXPECT_CALL(i2c, Read(ImtqAddress, _)).WillOnce(Invoke([](uint8_t /*address*/, auto outData) {
            EXPECT_EQ(outData.size(), 2);
            outData[0] = 0x02;
            outData[1] = 0;
            return I2CResult::Failure;
        }));

        auto status = imtq.SendNoOperation();
        EXPECT_FALSE(status);
        EXPECT_EQ(imtq.LastError, ImtqDriverError::I2CReadFailed);
        EXPECT_EQ(error_counter, 5);
    }

    TEST_F(ImtqTest, SoftwareReset_Ok)
    {
        EXPECT_CALL(i2c, Write(ImtqAddress, ElementsAre(0xAA))).WillOnce(Return(I2CResult::OK));
        EXPECT_CALL(i2c, Read(ImtqAddress, _)).WillOnce(Return(I2CResult::Nack));

        auto status = imtq.SoftwareReset();
        EXPECT_TRUE(status);
        EXPECT_EQ(error_counter, 0);
    }

    TEST_F(ImtqTest, SoftwareReset_FastBootDelayOnRead)
    {
        // fast boot/delay on read
        EXPECT_CALL(i2c, Write(ImtqAddress, ElementsAre(0xAA))).WillOnce(Return(I2CResult::OK));
        EXPECT_CALL(i2c, Read(ImtqAddress, _)).WillOnce(Invoke([](uint8_t /*address*/, auto outData) {
            EXPECT_EQ(outData.size(), 2);
            outData[0] = 0xFF;
            outData[1] = 0xFF;
            return I2CResult::OK;
        }));

        auto status = imtq.SoftwareReset();
        EXPECT_TRUE(status);
        EXPECT_EQ(error_counter, 0);
    }

    TEST_F(ImtqTest, SoftwareReset_I2CReadReturnedFail)
    {
        EXPECT_CALL(i2c, Write(ImtqAddress, ElementsAre(0xAA))).WillOnce(Return(I2CResult::OK));
        EXPECT_CALL(i2c, Read(ImtqAddress, _)).WillOnce(Return(I2CResult::Failure));

        auto status = imtq.SoftwareReset();
        EXPECT_FALSE(status);
        EXPECT_EQ(error_counter, 5);
    }

    TEST_F(ImtqTest, SoftwareReset_I2CWriteReturnedFail)
    {
        EXPECT_CALL(i2c, Write(ImtqAddress, ElementsAre(0xAA))).WillOnce(Return(I2CResult::Nack));

        auto status = imtq.SoftwareReset();
        EXPECT_FALSE(status);
        EXPECT_EQ(error_counter, 5);
    }

    TEST_F(ImtqTest, SoftwareReset_ResetRejected)
    {
        EXPECT_CALL(i2c, Write(ImtqAddress, ElementsAre(0xAA))).WillOnce(Return(I2CResult::OK));
        EXPECT_CALL(i2c, Read(ImtqAddress, _)).WillOnce(Invoke([](uint8_t /*address*/, auto outData) {
            EXPECT_EQ(outData.size(), 2);
            outData[0] = 0xAA;
            outData[1] = 1;
            return I2CResult::OK;
        }));

        auto status = imtq.SoftwareReset();
        EXPECT_FALSE(status);
        EXPECT_EQ(error_counter, 0);
    }

    TEST_F(ImtqTest, CancelOperation)
    {
        EXPECT_CALL(i2c, Write(ImtqAddress, ElementsAre(0x03))).WillOnce(Return(I2CResult::OK));
        EXPECT_CALL(i2c, Read(ImtqAddress, _)).WillOnce(Invoke([](uint8_t /*address*/, auto outData) {
            outData[0] = 0x03;
            outData[1] = 0;
            return I2CResult::OK;
        }));

        EXPECT_TRUE(imtq.CancelOperation());
        EXPECT_EQ(error_counter, 0);
    }

    TEST_F(ImtqTest, StartMTMMeasurement)
    {
        EXPECT_CALL(i2c, Write(ImtqAddress, ElementsAre(0x04))).WillOnce(Return(I2CResult::OK));
        EXPECT_CALL(i2c, Read(ImtqAddress, _)).WillOnce(Invoke([](uint8_t /*address*/, auto outData) {
            EXPECT_EQ(outData.size(), 2);
            outData[0] = 0x04;
            outData[1] = 0;
            return I2CResult::OK;
        }));

        EXPECT_TRUE(imtq.StartMTMMeasurement());
        EXPECT_EQ(error_counter, 0);
    }

    TEST_F(ImtqTest, StartAllAxisSelfTest)
    {
        EXPECT_CALL(i2c, Write(ImtqAddress, ElementsAre(0x08, 0x00))).WillOnce(Return(I2CResult::OK));

        EXPECT_CALL(i2c, Read(ImtqAddress, _)).WillOnce(Invoke([](uint8_t /*address*/, auto outData) {
            EXPECT_EQ(outData.size(), 2);

            outData[0] = 0x08;
            outData[1] = 0;
            return I2CResult::OK;
        }));

        auto status = imtq.StartAllAxisSelfTest();
        EXPECT_TRUE(status);
        EXPECT_EQ(error_counter, 0);
    }

    inline bool operator==(const SelfTestResult::StepResult a, const SelfTestResult::StepResult b)
    {
        if (a.error.GetValue() == b.error.GetValue() && a.actualStep == b.actualStep &&
            a.RawMagnetometerMeasurement == b.RawMagnetometerMeasurement &&
            a.CalibratedMagnetometerMeasurement == b.CalibratedMagnetometerMeasurement && a.CoilCurrent == b.CoilCurrent &&
            a.CoilTemperature == b.CoilTemperature)
            return true;
        return false;
    }
}
