#include <chrono>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gmock/gmock-matchers.h"
#include "I2C/I2CMock.hpp"
#include "OsMock.hpp"
#include "base/reader.h"
#include "base/writer.h"
#include "i2c/i2c.h"
#include "mock/InterruptPinDriverMock.hpp"
#include "mock/error_counter.hpp"
#include "os/os.hpp"
#include "system.h"
#include "utils.hpp"

#include "suns/suns.hpp"

using testing::_;
using testing::Eq;
using testing::Return;
using testing::Invoke;
using testing::ElementsAre;
using drivers::i2c::I2CResult;
using devices::suns::MeasurementData;
using devices::suns::OperationStatus;

using namespace devices::suns;

namespace
{
    static const uint8_t SunSAddress = 0x44;

    class SunSTest : public testing::Test
    {
      protected:
        SunSTest();

        testing::NiceMock<ErrorCountingConfigrationMock> errorsConfig;
        error_counter::ErrorCounting errors;
        devices::suns::SunSDriver suns;
        I2CBusMock i2c;
        InterruptPinDriverMock pinDriver;
        OSMock os;
        OSReset reset;

        devices::suns::SunSDriver::ErrorCounter error_counter;
    };

    SunSTest::SunSTest() : errors{errorsConfig}, suns{errors, i2c, pinDriver}, error_counter{errors}
    {
        this->reset = InstallProxy(&os);
    }

    TEST_F(SunSTest, BadOpcodeResponse)
    {
        ON_CALL(os, TakeSemaphore(_, _)).WillByDefault(Return(OSResult::Success));
        ON_CALL(os, GiveSemaphore(_)).WillByDefault(Return(OSResult::Success));

        EXPECT_CALL(i2c, Write(SunSAddress, ElementsAre(0x80, 0x01, 0x02))).WillOnce(Return(I2CResult::OK));
        EXPECT_CALL(i2c, Read(SunSAddress, _)).WillOnce(Invoke([](uint8_t /*address*/, auto outData) {
            EXPECT_EQ(outData.size(), 536);
            outData[0] = 0x01;
            outData[1] = 0x11;
            return I2CResult::OK;
        }));

        MeasurementData data;
        auto status = suns.MeasureSunS(data, 1, 2);

        ASSERT_THAT(status, Eq(OperationStatus::WrongOpcodeInResponse));
        ASSERT_THAT(error_counter, Eq(0));
    }

    TEST_F(SunSTest, BadWhoAmIResponse)
    {
        ON_CALL(os, TakeSemaphore(_, _)).WillByDefault(Return(OSResult::Success));
        ON_CALL(os, GiveSemaphore(_)).WillByDefault(Return(OSResult::Success));

        EXPECT_CALL(i2c, Write(SunSAddress, ElementsAre(0x80, 0x01, 0x02))).WillOnce(Return(I2CResult::OK));
        EXPECT_CALL(i2c, Read(SunSAddress, _)).WillOnce(Invoke([](uint8_t /*address*/, auto outData) {
            EXPECT_EQ(outData.size(), 536);
            outData[0] = 0x80;
            outData[1] = 0x01;
            return I2CResult::OK;
        }));

        MeasurementData data;
        auto status = suns.MeasureSunS(data, 1, 2);

        ASSERT_THAT(status, Eq(OperationStatus::WhoAmIMismatch));
        ASSERT_THAT(error_counter, Eq(0));
    }

    TEST_F(SunSTest, I2CWriteFail)
    {
        ON_CALL(os, TakeSemaphore(_, _)).WillByDefault(Return(OSResult::Success));
        ON_CALL(os, GiveSemaphore(_)).WillByDefault(Return(OSResult::Success));

        EXPECT_CALL(i2c, Write(SunSAddress, ElementsAre(0x80, 0x01, 0x02))).WillOnce(Return(I2CResult::Failure));

        MeasurementData data;
        auto status = suns.MeasureSunS(data, 1, 2);

        ASSERT_THAT(status, Eq(OperationStatus::I2CWriteFailed));
        ASSERT_THAT(error_counter, Eq(5));
    }

    TEST_F(SunSTest, I2CReadFail)
    {
        ON_CALL(os, TakeSemaphore(_, _)).WillByDefault(Return(OSResult::Success));
        ON_CALL(os, GiveSemaphore(_)).WillByDefault(Return(OSResult::Success));

        EXPECT_CALL(i2c, Write(SunSAddress, ElementsAre(0x80, 0x01, 0x02))).WillOnce(Return(I2CResult::OK));
        EXPECT_CALL(i2c, Read(SunSAddress, _)).WillOnce(Invoke([](uint8_t /*address*/, auto outData) {
            EXPECT_EQ(outData.size(), 536);
            outData[0] = 0x80;
            outData[1] = 0x11;
            return I2CResult::Failure;
        }));

        MeasurementData data;
        auto status = suns.MeasureSunS(data, 1, 2);

        ASSERT_THAT(status, Eq(OperationStatus::I2CReadFailed));
        ASSERT_THAT(error_counter, Eq(5));
    }

    TEST_F(SunSTest, StartMeasurement)
    {
        ON_CALL(os, TakeSemaphore(_, _)).WillByDefault(Return(OSResult::Success));
        ON_CALL(os, GiveSemaphore(_)).WillByDefault(Return(OSResult::Success));

        EXPECT_CALL(i2c, Write(SunSAddress, ElementsAre(0x80, 0x01, 0x02))).WillOnce(Return(I2CResult::OK));

        auto status = suns.StartMeasurement(1, 2);

        ASSERT_THAT(status, Eq(OperationStatus::OK));
        ASSERT_THAT(error_counter, Eq(0));
    }

    TEST_F(SunSTest, GetMeasuredData)
    {
        ON_CALL(os, TakeSemaphore(_, _)).WillByDefault(Return(OSResult::Success));
        ON_CALL(os, GiveSemaphore(_)).WillByDefault(Return(OSResult::Success));

        EXPECT_CALL(i2c, Read(SunSAddress, _)).WillOnce(Invoke([](uint8_t /*address*/, auto outData) {
            EXPECT_EQ(outData.size(), 536);
            Writer writer{outData};
            writer.WriteByte(0x80);
            writer.WriteByte(0x11);
            writer.WriteWordLE(0x0102);
            writer.WriteWordLE(0x0304);
            writer.WriteWordLE(0x0506);

            writer.WriteWordLE(0x0708);
            writer.WriteWordLE(0x090A);
            writer.WriteWordLE(0x0B0C);
            writer.WriteWordLE(0x0D0E);

            writer.WriteWordLE(0x0F10);
            writer.WriteWordLE(0x1112);
            writer.WriteWordLE(0x1314);
            writer.WriteWordLE(0x1516);

            writer.WriteWordLE(0x1718);
            writer.WriteWordLE(0x191A);
            writer.WriteWordLE(0x1B1C);
            writer.WriteWordLE(0x1D1E);

            writer.WriteWordLE(0x1F20);

            writer.WriteWordLE(0x2122);
            writer.WriteWordLE(0x2324);
            writer.WriteWordLE(0x2526);
            writer.WriteWordLE(0x2728);

            writer.WriteByte(0x29);
            writer.WriteByte(0x2A);

            writer.WriteWordLE(0x2B2C);
            writer.WriteWordLE(0x2D2E);
            writer.WriteWordLE(0x2F30);
            writer.WriteWordLE(0x3132);

            writer.WriteWordLE(0x3334);
            writer.WriteWordLE(0x3536);
            writer.WriteWordLE(0x3738);
            writer.WriteWordLE(0x393A);

            writer.WriteWordLE(0x3B3C);
            writer.WriteWordLE(0x3D3E);
            writer.WriteWordLE(0x3F40);
            writer.WriteWordLE(0x4142);

            return I2CResult::OK;
        }));

        MeasurementData data;
        auto status = suns.GetMeasuredData(data);

        ASSERT_THAT(status, Eq(OperationStatus::OK));
        ASSERT_THAT(error_counter, Eq(0));

        ASSERT_THAT(data.status.ack, Eq(0x0102));
        ASSERT_THAT(data.status.presence, Eq(0x0304));
        ASSERT_THAT(data.status.adc_valid, Eq(0x0506));
        ASSERT_THAT(data.visible_light[0], ElementsAre(0x0708, 0x090A, 0x0B0C, 0x0D0E));
        ASSERT_THAT(data.visible_light[1], ElementsAre(0x0F10, 0x1112, 0x1314, 0x1516));
        ASSERT_THAT(data.visible_light[2], ElementsAre(0x1718, 0x191A, 0x1B1C, 0x1D1E));
        ASSERT_THAT(data.temperature.structure, Eq(0x1F20));
        ASSERT_THAT(data.temperature.panels, ElementsAre(0x2122, 0x2324, 0x2526, 0x2728));
        ASSERT_THAT(data.parameters.gain, Eq(0x29));
        ASSERT_THAT(data.parameters.itime, Eq(0x2A));
        ASSERT_THAT(data.infrared[0], ElementsAre(0x2B2C, 0x2D2E, 0x2F30, 0x3132));
        ASSERT_THAT(data.infrared[1], ElementsAre(0x3334, 0x3536, 0x3738, 0x393A));
        ASSERT_THAT(data.infrared[2], ElementsAre(0x3B3C, 0x3D3E, 0x3F40, 0x4142));
    }
    
    TEST_F(SunSTest, IRQHandlerActiveTest)
    {
        EXPECT_CALL(pinDriver, ClearInterrupt()).Times(1);
        EXPECT_CALL(pinDriver, Value()).Times(1);
        EXPECT_CALL(os, GiveSemaphoreISR(_)).Times(1);
        EXPECT_CALL(os, EndSwitchingISR()).Times(1);

        pinDriver.SetValue(false);
        suns.IRQHandler();
        EXPECT_EQ(error_counter, 0);
    }

    TEST_F(SunSTest, IRQHandlerInactiveTest)
    {
        EXPECT_CALL(pinDriver, ClearInterrupt()).Times(1);
        EXPECT_CALL(pinDriver, Value()).Times(1);
        EXPECT_CALL(os, GiveSemaphoreISR(_)).Times(0);
        EXPECT_CALL(os, EndSwitchingISR()).Times(1);

        pinDriver.SetValue(true);
        suns.IRQHandler();
        EXPECT_EQ(error_counter, 0);
    }

    TEST_F(SunSTest, IsBusyLine)
    {
        EXPECT_CALL(pinDriver, Value()).Times(2);

        pinDriver.SetValue(true);
        ASSERT_THAT(suns.IsBusy(), Eq(true));
        EXPECT_EQ(error_counter, 0);

        pinDriver.SetValue(false);
        ASSERT_THAT(suns.IsBusy(), Eq(false));
        EXPECT_EQ(error_counter, 0);
    }

    TEST_F(SunSTest, WaitForDataSuccesful)
    {
        EXPECT_CALL(os, TakeSemaphore(_, _))
            .Times(1)
            .WillOnce(Invoke([=](OSSemaphoreHandle /*syncs*/, std::chrono::milliseconds /*timeout*/) { return OSResult::Success; }));

        ASSERT_THAT(suns.WaitForData(), Eq(OSResult::Success));
        EXPECT_EQ(error_counter, 0);
    }

    TEST_F(SunSTest, WaitForDataTimeout)
    {
        EXPECT_CALL(os, TakeSemaphore(_, _))
            .Times(1)
            .WillOnce(Invoke([=](OSSemaphoreHandle /*syncs*/, std::chrono::milliseconds /*timeout*/) { return OSResult::Timeout; }));

        ASSERT_THAT(suns.WaitForData(), Eq(OSResult::Timeout));
        EXPECT_EQ(error_counter, 5);
    }

    TEST_F(SunSTest, SetingTimeout)
    {
        auto timeout = std::chrono::milliseconds(1000);
        EXPECT_CALL(os, TakeSemaphore(_, Eq(timeout))).Times(1);
        suns.SetDataTimeout(timeout);
        suns.WaitForData();
        EXPECT_EQ(error_counter, 0);
    }
}
