#include "antenna/miniport.h"
#include <cstdint>
#include <gsl/span>
#include <tuple>
#include "gtest/gtest.h"
#include "I2C/I2CMock.hpp"
#include "antenna/antenna.h"
#include "error_counter/error_counter.hpp"
#include "time/TimeSpan.hpp"
#include "utils.hpp"

static constexpr std::uint8_t Reset = 0xaa;
static constexpr std::uint8_t QueryTemperature = 0xc0;
static constexpr std::uint8_t QueryDeploymentStatus = 0xc3;

using testing::Return;
using testing::_;
using testing::Eq;
using testing::Ge;
using testing::Ne;
using testing::Invoke;
using testing::ElementsAre;
using gsl::span;
using drivers::i2c::I2CResult;
using namespace std::chrono_literals;
namespace
{
    class AntennaMiniportTest : public testing::Test
    {
      protected:
        AntennaMiniportTest();
        I2CBusMock i2c;
        AntennaMiniportDriver miniport;
        error_counter::AggregatedErrorCounter error;
    };

    AntennaMiniportTest::AntennaMiniportTest()
    {
    }

    TEST_F(AntennaMiniportTest, TestHardwareReset)
    {
        i2c.ExpectWriteCommand(ANTENNA_PRIMARY_CHANNEL, Reset).WillOnce(Return(I2CResult::OK));
        const auto status = miniport.Reset(error, &i2c, ANTENNA_PRIMARY_CHANNEL);
        ASSERT_THAT(status, Eq(OSResult::Success));
        ASSERT_THAT(error.GetAggregatedResult(), Eq(true));
    }

    TEST_F(AntennaMiniportTest, TestHardwareResetFailure)
    {
        i2c.ExpectWriteCommand(ANTENNA_PRIMARY_CHANNEL, Reset).WillOnce(Return(I2CResult::Nack));
        const auto status = miniport.Reset(error, &i2c, ANTENNA_PRIMARY_CHANNEL);
        ASSERT_THAT(status, Ne(OSResult::Success));
        ASSERT_THAT(error.GetAggregatedResult(), Eq(false));
    }

    TEST_F(AntennaMiniportTest, TestAntennaTemperature)
    {
        EXPECT_CALL(i2c, WriteRead(ANTENNA_PRIMARY_CHANNEL, ElementsAre(QueryTemperature), _))
            .WillOnce(Invoke([=](uint8_t /*address*/, span<const uint8_t> /*inData*/, span<uint8_t> outData) {
                std::fill(outData.begin(), outData.end(), 0);
                outData[0] = 0;
                outData[1] = 0x11;
                return I2CResult::OK;
            }));
        uint16_t response = 0;
        const auto status = miniport.GetTemperature(error, &i2c, ANTENNA_PRIMARY_CHANNEL, &response);
        ASSERT_THAT(status, Eq(OSResult::Success));
        ASSERT_THAT(response, Eq(0x11));
        ASSERT_THAT(error.GetAggregatedResult(), Eq(true));
    }

    TEST_F(AntennaMiniportTest, TestAntennaTemperatureOutOfRange)
    {
        EXPECT_CALL(i2c, WriteRead(ANTENNA_PRIMARY_CHANNEL, ElementsAre(QueryTemperature), _))
            .WillOnce(Invoke([=](uint8_t /*address*/, span<const uint8_t> /*inData*/, span<uint8_t> outData) {
                std::fill(outData.begin(), outData.end(), 0);
                outData[0] = 0xfc;
                outData[1] = 0;
                return I2CResult::OK;
            }));
        uint16_t response;
        const auto status = miniport.GetTemperature(error, &i2c, ANTENNA_PRIMARY_CHANNEL, &response);
        ASSERT_THAT(status, Eq(OSResult::Success));
        ASSERT_THAT(response, Eq(0));
        ASSERT_THAT(error.GetAggregatedResult(), Eq(true));
    }

    TEST_F(AntennaMiniportTest, TestAntennaTemperatureFailure)
    {
        EXPECT_CALL(i2c, WriteRead(ANTENNA_PRIMARY_CHANNEL, ElementsAre(QueryTemperature), _)).WillOnce(Return(I2CResult::Nack));
        uint16_t response;
        const auto status = miniport.GetTemperature(error, &i2c, ANTENNA_PRIMARY_CHANNEL, &response);
        ASSERT_THAT(status, Ne(OSResult::Success));
        ASSERT_THAT(response, Eq(0u));
        ASSERT_THAT(error.GetAggregatedResult(), Eq(false));
    }

    class AntennaDeploymentStatusTest
        : public testing::TestWithParam<
              std::tuple<uint8_t, uint8_t, I2CResult, OSResult, uint32_t, uint32_t, uint32_t, bool, bool, bool, bool>>
    {
      public:
        AntennaDeploymentStatusTest();
        void MockI2C();

      protected:
        I2CBusMock i2c;
        AntennaMiniportDriver miniport;
        error_counter::AggregatedErrorCounter error;
    };

    AntennaDeploymentStatusTest::AntennaDeploymentStatusTest()
    {
    }

    void AntennaDeploymentStatusTest::MockI2C()
    {
        const auto response1 = std::get<0>(GetParam());
        const auto response2 = std::get<1>(GetParam());
        const auto i2cResult = std::get<2>(GetParam());

        EXPECT_CALL(i2c, WriteRead(ANTENNA_PRIMARY_CHANNEL, ElementsAre(QueryDeploymentStatus), _))
            .WillOnce(Invoke([=](uint8_t /*address*/, span<const uint8_t> /*inData*/, span<uint8_t> outData) {
                std::fill(outData.begin(), outData.end(), 0);
                outData[0] = response1;
                outData[1] = response2;
                return i2cResult;
            }));
    }

    INSTANTIATE_TEST_CASE_P(AntennaDeploymentStatusTestSet,
        AntennaDeploymentStatusTest,
        testing::Values(std::make_tuple(0, 0, I2CResult::Nack, OSResult::IOError, 0u, 0u, 0u, false, false, false, false),
            std::make_tuple(0x44, 0x44, I2CResult::OK, OSResult::Success, 0x01010101u, 0u, 0x01010101u, false, false, false, true),
            std::make_tuple(0x41, 0x44, I2CResult::OK, OSResult::Success, 0x01010101u, 0u, 0x00010101u, false, true, false, true),
            std::make_tuple(0x04, 0x45, I2CResult::OK, OSResult::Success, 0x01010101u, 0u, 0x01000101u, true, false, false, true),
            std::make_tuple(0x46, 0x40, I2CResult::OK, OSResult::Success, 0x01010101u, 0x1000000, 0x01010001u, false, false, false, true),
            std::make_tuple(0x5C, 0x04, I2CResult::OK, OSResult::Success, 0x00010101u, 0u, 0x01010100u, false, false, true, true),
            std::make_tuple(0x4A, 0x44, I2CResult::OK, OSResult::Success, 0x00010101u, 0x1000000, 0x00010101u, false, false, false, true),
            std::make_tuple(0x20, 0x44, I2CResult::OK, OSResult::Success, 0x01010101u, 0x10000, 0x00000101u, false, false, false, true),
            std::make_tuple(0x80, 0x40, I2CResult::OK, OSResult::Success, 0x01000101u, 0u, 0x00000001u, false, false, false, true),
            std::make_tuple(0xF4, 0x44, I2CResult::OK, OSResult::Success, 0x01000101u, 0x10000, 0x01010101u, false, false, true, true),
            std::make_tuple(0x44, 0x46, I2CResult::OK, OSResult::Success, 0x01010101u, 0x100, 0x01010101u, false, false, false, true),
            std::make_tuple(0x44, 0x4C, I2CResult::OK, OSResult::Success, 0x01010001u, 0u, 0x01010101u, false, false, false, true),
            std::make_tuple(0x44, 0x4E, I2CResult::OK, OSResult::Success, 0x01010001u, 0x100, 0x01010101u, false, false, false, true),
            std::make_tuple(0x44, 0x64, I2CResult::OK, OSResult::Success, 0x01010101u, 0x1, 0x01010101u, false, false, false, true),
            std::make_tuple(0x44, 0xC4, I2CResult::OK, OSResult::Success, 0x01010100u, 0u, 0x01010101u, false, false, false, true),
            std::make_tuple(0x44, 0xE4, I2CResult::OK, OSResult::Success, 0x01010100u, 0x1, 0x01010101u, false, false, false, true),
            std::make_tuple(0x00, 0x10, I2CResult::OK, OSResult::Success, 0x01010101u, 0u, 0x0u, false, false, false, true)), );
}
