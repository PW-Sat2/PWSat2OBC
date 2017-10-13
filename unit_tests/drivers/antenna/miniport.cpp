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

static constexpr std::uint8_t DeployAntenna1 = 0xa1;
static constexpr std::uint8_t DeployAntenna2 = 0xa2;
static constexpr std::uint8_t DeployAntenna3 = 0xa3;
static constexpr std::uint8_t DeployAntenna4 = 0xa4;
static constexpr std::uint8_t StartDeployment = 0xa5;
static constexpr std::uint8_t CancelDeployment = 0xa9;
static constexpr std::uint8_t Reset = 0xaa;
static constexpr std::uint8_t Disarm = 0xac;
static constexpr std::uint8_t Arm = 0xad;
static constexpr std::uint8_t DeployAntenna1Override = 0xba;
static constexpr std::uint8_t DeployAntenna2Override = 0xbb;
static constexpr std::uint8_t DeployAntenna3Override = 0xbc;
static constexpr std::uint8_t DeployAntenna4Override = 0xbd;
static constexpr std::uint8_t QueryActivationCount1 = 0xb0;
static constexpr std::uint8_t QueryActivationCount2 = 0xb1;
static constexpr std::uint8_t QueryActivationCount3 = 0xb2;
static constexpr std::uint8_t QueryActivationCount4 = 0xb3;
static constexpr std::uint8_t QueryActivationTime1 = 0xb4;
static constexpr std::uint8_t QueryActivationTime2 = 0xb5;
static constexpr std::uint8_t QueryActivationTime3 = 0xb6;
static constexpr std::uint8_t QueryActivationTime4 = 0xb7;
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

    TEST_F(AntennaMiniportTest, TestArmingDeployment)
    {
        i2c.ExpectWriteCommand(ANTENNA_PRIMARY_CHANNEL, Arm).WillOnce(Return(I2CResult::OK));
        const auto status = miniport.ArmDeploymentSystem(error, &i2c, ANTENNA_PRIMARY_CHANNEL);
        ASSERT_THAT(status, Eq(OSResult::Success));
        ASSERT_THAT(error.GetAggregatedResult(), Eq(true));
    }

    TEST_F(AntennaMiniportTest, TestArmingDeploymentFailure)
    {
        i2c.ExpectWriteCommand(ANTENNA_PRIMARY_CHANNEL, Arm).WillOnce(Return(I2CResult::Nack));
        const auto status = miniport.ArmDeploymentSystem(error, &i2c, ANTENNA_PRIMARY_CHANNEL);
        ASSERT_THAT(status, Ne(OSResult::Success));
        ASSERT_THAT(error.GetAggregatedResult(), Eq(false));
    }

    TEST_F(AntennaMiniportTest, TestDisarmingDeployment)
    {
        i2c.ExpectWriteCommand(ANTENNA_PRIMARY_CHANNEL, Disarm).WillOnce(Return(I2CResult::OK));
        const auto status = miniport.DisarmDeploymentSystem(error, &i2c, ANTENNA_PRIMARY_CHANNEL);
        ASSERT_THAT(status, Eq(OSResult::Success));
        ASSERT_THAT(error.GetAggregatedResult(), Eq(true));
    }

    TEST_F(AntennaMiniportTest, TestDisarmingDeploymentFailure)
    {
        i2c.ExpectWriteCommand(ANTENNA_PRIMARY_CHANNEL, Disarm).WillOnce(Return(I2CResult::Nack));
        const auto status = miniport.DisarmDeploymentSystem(error, &i2c, ANTENNA_PRIMARY_CHANNEL);
        ASSERT_THAT(status, Ne(OSResult::Success));
        ASSERT_THAT(error.GetAggregatedResult(), Eq(false));
    }

    TEST_F(AntennaMiniportTest, TestAutomaticDeployment)
    {
        EXPECT_CALL(i2c, Write(ANTENNA_PRIMARY_CHANNEL, testing::ElementsAre(StartDeployment, 50))).WillOnce(Return(I2CResult::OK));
        const auto status = miniport.InitializeAutomaticDeployment(error, &i2c, ANTENNA_PRIMARY_CHANNEL, 200s);
        ASSERT_THAT(status, Eq(OSResult::Success));
        ASSERT_THAT(error.GetAggregatedResult(), Eq(true));
    }

    TEST_F(AntennaMiniportTest, TestAutomaticDeploymentFailure)
    {
        EXPECT_CALL(i2c, Write(ANTENNA_PRIMARY_CHANNEL, BeginsWith(StartDeployment))).WillOnce(Return(I2CResult::Nack));
        const auto status = miniport.InitializeAutomaticDeployment(error, &i2c, ANTENNA_PRIMARY_CHANNEL, 200s);
        ASSERT_THAT(status, Ne(OSResult::Success));
        ASSERT_THAT(error.GetAggregatedResult(), Eq(false));
    }

    TEST_F(AntennaMiniportTest, TestCancelAutomaticDeployment)
    {
        i2c.ExpectWriteCommand(ANTENNA_PRIMARY_CHANNEL, CancelDeployment).WillOnce(Return(I2CResult::OK));
        const auto status = miniport.CancelAntennaDeployment(error, &i2c, ANTENNA_PRIMARY_CHANNEL);
        ASSERT_THAT(status, Eq(OSResult::Success));
        ASSERT_THAT(error.GetAggregatedResult(), Eq(true));
    }

    TEST_F(AntennaMiniportTest, TestCancelAutomaticDeploymentFailure)
    {
        i2c.ExpectWriteCommand(ANTENNA_PRIMARY_CHANNEL, CancelDeployment).WillOnce(Return(I2CResult::Nack));
        const auto status = miniport.CancelAntennaDeployment(error, &i2c, ANTENNA_PRIMARY_CHANNEL);
        ASSERT_THAT(status, Ne(OSResult::Success));
        ASSERT_THAT(error.GetAggregatedResult(), Eq(false));
    }

    TEST_F(AntennaMiniportTest, TestManualAntennaDeployment)
    {
        EXPECT_CALL(i2c, Write(ANTENNA_PRIMARY_CHANNEL, ElementsAre(DeployAntenna1, 200u))).WillOnce(Return(I2CResult::OK));
        const auto status = miniport.DeployAntenna(error, &i2c, ANTENNA_PRIMARY_CHANNEL, ANTENNA1_ID, 200s, false);
        ASSERT_THAT(status, Eq(OSResult::Success));
        ASSERT_THAT(error.GetAggregatedResult(), Eq(true));
    }

    TEST_F(AntennaMiniportTest, TestManualAntennaDeploymentFailure)
    {
        EXPECT_CALL(i2c, Write(ANTENNA_PRIMARY_CHANNEL, ElementsAre(DeployAntenna2, 200u))).WillOnce(Return(I2CResult::Nack));
        const auto status = miniport.DeployAntenna(error, &i2c, ANTENNA_PRIMARY_CHANNEL, ANTENNA2_ID, 200s, false);
        ASSERT_THAT(status, Ne(OSResult::Success));
        ASSERT_THAT(error.GetAggregatedResult(), Eq(false));
    }

    TEST_F(AntennaMiniportTest, TestManualAntennaDeploymentWithOverride)
    {
        EXPECT_CALL(i2c, Write(ANTENNA_PRIMARY_CHANNEL, ElementsAre(DeployAntenna1Override, 200u))).WillOnce(Return(I2CResult::OK));
        const auto status = miniport.DeployAntenna(error, &i2c, ANTENNA_PRIMARY_CHANNEL, ANTENNA1_ID, 200s, true);
        ASSERT_THAT(status, Eq(OSResult::Success));
        ASSERT_THAT(error.GetAggregatedResult(), Eq(true));
    }

    TEST_F(AntennaMiniportTest, TestManualAntennaDeploymentWithOverrideFailure)
    {
        EXPECT_CALL(i2c, Write(ANTENNA_BACKUP_CHANNEL, ElementsAre(DeployAntenna3Override, 200u))).WillOnce(Return(I2CResult::Nack));
        const auto status = miniport.DeployAntenna(error, &i2c, ANTENNA_BACKUP_CHANNEL, ANTENNA3_ID, 200s, true);
        ASSERT_THAT(status, Ne(OSResult::Success));
        ASSERT_THAT(error.GetAggregatedResult(), Eq(false));
    }

    TEST_F(AntennaMiniportTest, TestAntennaActivationCount)
    {
        EXPECT_CALL(i2c, WriteRead(ANTENNA_PRIMARY_CHANNEL, ElementsAre(QueryActivationCount1), _))
            .WillOnce(Invoke([=](uint8_t /*address*/, span<const uint8_t> /*inData*/, span<uint8_t> outData) {
                std::fill(outData.begin(), outData.end(), 0);
                outData[0] = 10;
                return I2CResult::OK;
            }));
        uint8_t response;
        const auto status = miniport.GetAntennaActivationCount(error, &i2c, ANTENNA_PRIMARY_CHANNEL, ANTENNA1_ID, &response);
        ASSERT_THAT(status, Eq(OSResult::Success));
        ASSERT_THAT(response, Eq(10u));
        ASSERT_THAT(error.GetAggregatedResult(), Eq(true));
    }

    TEST_F(AntennaMiniportTest, TestAntennaActivationCountFailure)
    {
        EXPECT_CALL(i2c, WriteRead(ANTENNA_BACKUP_CHANNEL, ElementsAre(QueryActivationCount2), _)).WillOnce(Return(I2CResult::Nack));
        uint8_t response;
        const auto status = miniport.GetAntennaActivationCount(error, &i2c, ANTENNA_BACKUP_CHANNEL, ANTENNA2_ID, &response);
        ASSERT_THAT(status, Ne(OSResult::Success));
        ASSERT_THAT(response, Eq(0u));
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

    TEST_F(AntennaMiniportTest, TestAntennaActivationTime)
    {
        EXPECT_CALL(i2c, WriteRead(ANTENNA_PRIMARY_CHANNEL, ElementsAre(QueryActivationTime1), _))
            .WillOnce(Invoke([=](uint8_t /*address*/, span<const uint8_t> /*inData*/, span<uint8_t> outData) {
                std::fill(outData.begin(), outData.end(), 0);
                outData[1] = 10;
                return I2CResult::OK;
            }));
        std::chrono::milliseconds response;
        const auto status = miniport.GetAntennaActivationTime(error, &i2c, ANTENNA_PRIMARY_CHANNEL, ANTENNA1_ID, &response);
        ASSERT_THAT(status, Eq(OSResult::Success));
        ASSERT_THAT(response, Eq(128000ms));
        ASSERT_THAT(error.GetAggregatedResult(), Eq(true));
    }

    TEST_F(AntennaMiniportTest, TestAntennaActivationTimeFailure)
    {
        EXPECT_CALL(i2c, WriteRead(ANTENNA_PRIMARY_CHANNEL, ElementsAre(QueryActivationTime2), _)).WillOnce(Return(I2CResult::Nack));
        std::chrono::milliseconds response;
        const auto status = miniport.GetAntennaActivationTime(error, &i2c, ANTENNA_PRIMARY_CHANNEL, ANTENNA2_ID, &response);
        ASSERT_THAT(status, Ne(OSResult::Success));
        ASSERT_THAT(response, Eq(0ms));
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

    TEST_P(AntennaDeploymentStatusTest, TestGetDeploymentStatus)
    {
        MockI2C();

        AntennaDeploymentStatus response;
        const auto expectedResult = std::get<3>(GetParam());

        const auto result = miniport.GetDeploymentStatus(error, &i2c, ANTENNA_PRIMARY_CHANNEL, &response);
        ASSERT_THAT(result, Eq(expectedResult));
    }

    TEST_P(AntennaDeploymentStatusTest, TestDeploymentStatusData)
    {
        MockI2C();
        AntennaDeploymentStatus response;
        const auto deploymentStatuses = std::get<4>(GetParam());
        const auto deplomentActive = std::get<5>(GetParam());
        const auto timeLimitReached = std::get<6>(GetParam());
        const auto overrideActive = std::get<7>(GetParam());
        const auto systemArmed = std::get<8>(GetParam());
        const auto independentBurn = std::get<9>(GetParam());
        const auto expectedErrorResult = std::get<10>(GetParam());
        miniport.GetDeploymentStatus(error, &i2c, ANTENNA_PRIMARY_CHANNEL, &response);

        ASSERT_THAT(response.DeploymentStatus[0], Eq((deploymentStatuses & 0xff) != 0));
        ASSERT_THAT(response.DeploymentStatus[1], Eq((deploymentStatuses & 0xff00) != 0));
        ASSERT_THAT(response.DeploymentStatus[2], Eq((deploymentStatuses & 0xff0000) != 0));
        ASSERT_THAT(response.DeploymentStatus[3], Eq((deploymentStatuses & 0xff000000) != 0));

        ASSERT_THAT(response.IsDeploymentActive[0], Eq((deplomentActive & 0xff) != 0));
        ASSERT_THAT(response.IsDeploymentActive[1], Eq((deplomentActive & 0xff00) != 0));
        ASSERT_THAT(response.IsDeploymentActive[2], Eq((deplomentActive & 0xff0000) != 0));
        ASSERT_THAT(response.IsDeploymentActive[3], Eq((deplomentActive & 0xff000000) != 0));

        ASSERT_THAT(response.DeploymentTimeReached[0], Eq((timeLimitReached & 0xff) != 0));
        ASSERT_THAT(response.DeploymentTimeReached[1], Eq((timeLimitReached & 0xff00) != 0));
        ASSERT_THAT(response.DeploymentTimeReached[2], Eq((timeLimitReached & 0xff0000) != 0));
        ASSERT_THAT(response.DeploymentTimeReached[3], Eq((timeLimitReached & 0xff000000) != 0));

        ASSERT_THAT(response.IgnoringDeploymentSwitches, Eq(overrideActive));
        ASSERT_THAT(response.DeploymentSystemArmed, Eq(systemArmed));
        ASSERT_THAT(response.IsIndependentBurnActive, Eq(independentBurn));

        ASSERT_THAT(error.GetAggregatedResult(), Eq(expectedErrorResult));
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
