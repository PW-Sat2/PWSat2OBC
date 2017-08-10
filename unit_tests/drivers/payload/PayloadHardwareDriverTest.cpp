#include "gtest/gtest.h"
#include "I2C/I2CMock.hpp"
#include "OsMock.hpp"
#include "mock/InterruptPinDriverMock.hpp"
#include "mock/error_counter.hpp"
#include "payload/payload.h"

using namespace devices::payload;

using testing::Return;
using testing::_;
using testing::Eq;
using testing::Ge;
using testing::Ne;
using testing::Invoke;
using testing::ElementsAre;
using gsl::span;
using drivers::i2c::I2CResult;

namespace
{
    class PayloadHardwareDriverTest : public testing::Test
    {
      protected:
        PayloadHardwareDriverTest();

        testing::NiceMock<ErrorCountingConfigrationMock> errorsConfig;
        error_counter::ErrorCounting errors;

        I2CBusMock i2c;
        InterruptPinDriverMock pinDriver;
        PayloadDriver driver;

        OSMock os;
        OSReset reset;
        PayloadDriver::ErrorCounter error_counter;
    };

    PayloadHardwareDriverTest::PayloadHardwareDriverTest() : errors{errorsConfig}, driver{errors, i2c, pinDriver}, error_counter{errors}
    {
        this->reset = InstallProxy(&os);
    }

    TEST_F(PayloadHardwareDriverTest, WriteReadSuccessful)
    {
        EXPECT_CALL(i2c, WriteRead(PayloadDriver::I2CAddress, ElementsAre(2), _))
            .WillOnce(Invoke([=](uint8_t /*address*/, span<const uint8_t> /*inData*/, span<uint8_t> outData) {
                std::fill(outData.begin(), outData.end(), 0x20);
                return I2CResult::OK;
            }));

        std::array<uint8_t, 1> output = {2};
        std::array<uint8_t, 4> input;
        std::array<uint8_t, 4> reference = {0x20, 0x20, 0x20, 0x20};

        ASSERT_THAT(driver.PayloadRead(output, input), Eq(OSResult::Success));
        ASSERT_THAT(input, Eq(reference));
        EXPECT_EQ(error_counter, 0);
    }

    TEST_F(PayloadHardwareDriverTest, WriteReadFailed)
    {
        EXPECT_CALL(i2c, WriteRead(PayloadDriver::I2CAddress, ElementsAre(2), _))
            .WillOnce(Invoke([=](uint8_t /*address*/, span<const uint8_t> /*inData*/, span<uint8_t> outData) {
                std::fill(outData.begin(), outData.end(), 0x20);
                return I2CResult::ClockAlreadyLatched;
            }));

        std::array<uint8_t, 1> output = {2};
        std::array<uint8_t, 4> input;

        ASSERT_THAT(driver.PayloadRead(output, input), Eq(OSResult::InvalidOperation));
        EXPECT_EQ(error_counter, 5);
    }

    TEST_F(PayloadHardwareDriverTest, WriteSuccessful)
    {
        EXPECT_CALL(i2c, Write(PayloadDriver::I2CAddress, ElementsAre(2)))
            .WillOnce(Invoke([=](uint8_t /*address*/, span<const uint8_t> /*inData*/) { return I2CResult::OK; }));

        std::array<uint8_t, 1> output = {2};

        ASSERT_THAT(driver.PayloadWrite(output), Eq(OSResult::Success));
        EXPECT_EQ(error_counter, 0);
    }

    TEST_F(PayloadHardwareDriverTest, WriteFailed)
    {
        EXPECT_CALL(i2c, Write(PayloadDriver::I2CAddress, ElementsAre(2)))
            .WillOnce(Invoke([=](uint8_t /*address*/, span<const uint8_t> /*inData*/) { return I2CResult::Timeout; }));

        std::array<uint8_t, 1> output = {2};

        ASSERT_THAT(driver.PayloadWrite(output), Eq(OSResult::InvalidOperation));
        EXPECT_EQ(error_counter, 5);
    }

    TEST_F(PayloadHardwareDriverTest, IRQHandlerActiveTest)
    {
        EXPECT_CALL(pinDriver, Value()).Times(1);
        EXPECT_CALL(os, GiveSemaphoreISR(_)).Times(1);

        pinDriver.SetValue(false);
        driver.IRQHandler();
        EXPECT_EQ(error_counter, 0);
    }

    TEST_F(PayloadHardwareDriverTest, IRQHandlerInactiveTest)
    {
        EXPECT_CALL(pinDriver, Value()).Times(1);
        EXPECT_CALL(os, GiveSemaphoreISR(_)).Times(0);

        pinDriver.SetValue(true);
        driver.IRQHandler();
        EXPECT_EQ(error_counter, 0);
    }

    TEST_F(PayloadHardwareDriverTest, IsBusyLine)
    {
        EXPECT_CALL(pinDriver, Value()).Times(2);

        pinDriver.SetValue(true);
        ASSERT_THAT(driver.IsBusy(), Eq(true));
        EXPECT_EQ(error_counter, 0);

        pinDriver.SetValue(false);
        ASSERT_THAT(driver.IsBusy(), Eq(false));
        EXPECT_EQ(error_counter, 0);
    }

    TEST_F(PayloadHardwareDriverTest, WaitForDataSuccesful)
    {
        EXPECT_CALL(os, TakeSemaphore(_, _))
            .Times(1)
            .WillOnce(Invoke([=](OSSemaphoreHandle /*syncs*/, std::chrono::milliseconds /*timeout*/) { return OSResult::Success; }));

        ASSERT_THAT(driver.WaitForData(), Eq(OSResult::Success));
        EXPECT_EQ(error_counter, 0);
    }

    TEST_F(PayloadHardwareDriverTest, WaitForDataTimeout)
    {
        EXPECT_CALL(os, TakeSemaphore(_, _))
            .Times(1)
            .WillOnce(Invoke([=](OSSemaphoreHandle /*syncs*/, std::chrono::milliseconds /*timeout*/) { return OSResult::Timeout; }));

        ASSERT_THAT(driver.WaitForData(), Eq(OSResult::Timeout));
        EXPECT_EQ(error_counter, 5);
    }

    TEST_F(PayloadHardwareDriverTest, SettingTimeout)
    {
        auto timeout = std::chrono::milliseconds(1000);
        EXPECT_CALL(os, TakeSemaphore(_, Eq(timeout))).Times(1);
        driver.SetDataTimeout(timeout);
        driver.WaitForData();
        EXPECT_EQ(error_counter, 0);
    }
}
