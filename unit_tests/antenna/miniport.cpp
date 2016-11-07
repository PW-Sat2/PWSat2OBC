#include "antenna/miniport.h"
#include <cstdint>
#include <tuple>
#include "gtest/gtest.h"
#include "antenna/antenna.h"
#include "i2c/I2CMock.hpp"
#include "time/TimeSpan.hpp"

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

class AntennaMiniportTest : public testing::Test
{
  protected:
    AntennaMiniportTest();
    I2CBusMock i2c;
    AntennaMiniportDriver miniport;
};

AntennaMiniportTest::AntennaMiniportTest()
{
    AntennaMiniportInitialize(&miniport);
}

TEST_F(AntennaMiniportTest, TestHardwareReset)
{
    EXPECT_CALL(i2c, I2CWrite(ANTENNA_PRIMARY_CHANNEL, Reset, _, 1)).WillOnce(Return(I2CResultOK));
    const auto status = miniport.Reset(&miniport, &i2c, ANTENNA_PRIMARY_CHANNEL);
    ASSERT_THAT(status, Eq(OSResultSuccess));
}

TEST_F(AntennaMiniportTest, TestHardwareResetFailure)
{
    EXPECT_CALL(i2c, I2CWrite(ANTENNA_PRIMARY_CHANNEL, Reset, _, 1)).WillOnce(Return(I2CResultNack));
    const auto status = miniport.Reset(&miniport, &i2c, ANTENNA_PRIMARY_CHANNEL);
    ASSERT_THAT(status, Ne(OSResultSuccess));
}

TEST_F(AntennaMiniportTest, TestArmingDeployment)
{
    EXPECT_CALL(i2c, I2CWrite(ANTENNA_PRIMARY_CHANNEL, Arm, _, 1)).WillOnce(Return(I2CResultOK));
    const auto status = miniport.ArmDeploymentSystem(&miniport, &i2c, ANTENNA_PRIMARY_CHANNEL);
    ASSERT_THAT(status, Eq(OSResultSuccess));
}

TEST_F(AntennaMiniportTest, TestArmingDeploymentFailure)
{
    EXPECT_CALL(i2c, I2CWrite(ANTENNA_PRIMARY_CHANNEL, Arm, _, 1)).WillOnce(Return(I2CResultNack));
    const auto status = miniport.ArmDeploymentSystem(&miniport, &i2c, ANTENNA_PRIMARY_CHANNEL);
    ASSERT_THAT(status, Ne(OSResultSuccess));
}

TEST_F(AntennaMiniportTest, TestDisarmingDeployment)
{
    EXPECT_CALL(i2c, I2CWrite(ANTENNA_PRIMARY_CHANNEL, Disarm, _, 1)).WillOnce(Return(I2CResultOK));
    const auto status = miniport.DisarmDeploymentSystem(&miniport, &i2c, ANTENNA_PRIMARY_CHANNEL);
    ASSERT_THAT(status, Eq(OSResultSuccess));
}

TEST_F(AntennaMiniportTest, TestDisarmingDeploymentFailure)
{
    EXPECT_CALL(i2c, I2CWrite(ANTENNA_PRIMARY_CHANNEL, Disarm, _, 1)).WillOnce(Return(I2CResultNack));
    const auto status = miniport.DisarmDeploymentSystem(&miniport, &i2c, ANTENNA_PRIMARY_CHANNEL);
    ASSERT_THAT(status, Ne(OSResultSuccess));
}

TEST_F(AntennaMiniportTest, TestAutomaticDeployment)
{
    EXPECT_CALL(i2c, I2CWrite(ANTENNA_PRIMARY_CHANNEL, StartDeployment, _, 1)).WillOnce(Return(I2CResultOK));
    const auto status = miniport.InitializeAutomaticDeployment(&miniport, &i2c, ANTENNA_PRIMARY_CHANNEL);
    ASSERT_THAT(status, Eq(OSResultSuccess));
}

TEST_F(AntennaMiniportTest, TestAutomaticDeploymentFailure)
{
    EXPECT_CALL(i2c, I2CWrite(ANTENNA_PRIMARY_CHANNEL, StartDeployment, _, 1)).WillOnce(Return(I2CResultNack));
    const auto status = miniport.InitializeAutomaticDeployment(&miniport, &i2c, ANTENNA_PRIMARY_CHANNEL);
    ASSERT_THAT(status, Ne(OSResultSuccess));
}

TEST_F(AntennaMiniportTest, TestCancelAutomaticDeployment)
{
    EXPECT_CALL(i2c, I2CWrite(ANTENNA_PRIMARY_CHANNEL, CancelDeployment, _, 1)).WillOnce(Return(I2CResultOK));
    const auto status = miniport.CancelAntennaDeployment(&miniport, &i2c, ANTENNA_PRIMARY_CHANNEL);
    ASSERT_THAT(status, Eq(OSResultSuccess));
}

TEST_F(AntennaMiniportTest, TestCancelAutomaticDeploymentFailure)
{
    EXPECT_CALL(i2c, I2CWrite(ANTENNA_PRIMARY_CHANNEL, CancelDeployment, _, 1)).WillOnce(Return(I2CResultNack));
    const auto status = miniport.CancelAntennaDeployment(&miniport, &i2c, ANTENNA_PRIMARY_CHANNEL);
    ASSERT_THAT(status, Ne(OSResultSuccess));
}

TEST_F(AntennaMiniportTest, TestManualAntennaDeployment)
{
    EXPECT_CALL(i2c, I2CWrite(ANTENNA_PRIMARY_CHANNEL, DeployAntenna1, _, 2))
        .WillOnce(Invoke([](const I2CAddress /*address*/,
            uint8_t /*command*/,
            const uint8_t* inData,
            size_t length //
            ) {
            EXPECT_THAT(length, Eq(2u));
            EXPECT_THAT(inData[1], Eq(200u));
            return I2CResultOK;
        }));
    const auto status = miniport.DeployAntenna(&miniport, &i2c, ANTENNA_PRIMARY_CHANNEL, ANTENNA1_ID, TimeSpanFromSeconds(200), false);
    ASSERT_THAT(status, Eq(OSResultSuccess));
}

TEST_F(AntennaMiniportTest, TestManualAntennaDeploymentFailure)
{
    EXPECT_CALL(i2c, I2CWrite(ANTENNA_PRIMARY_CHANNEL, DeployAntenna2, _, _)).WillOnce(Return(I2CResultNack));
    const auto status = miniport.DeployAntenna(&miniport, &i2c, ANTENNA_PRIMARY_CHANNEL, ANTENNA2_ID, TimeSpanFromMilliseconds(200), false);
    ASSERT_THAT(status, Ne(OSResultSuccess));
}

TEST_F(AntennaMiniportTest, TestManualAntennaDeploymentWithOverride)
{
    EXPECT_CALL(i2c, I2CWrite(ANTENNA_PRIMARY_CHANNEL, DeployAntenna1Override, _, 2))
        .WillOnce(Invoke([](const I2CAddress /*address*/,
            uint8_t /*command*/,
            const uint8_t* inData,
            size_t length //
            ) {
            EXPECT_THAT(length, Eq(2u));
            EXPECT_THAT(inData[1], Eq(200u));
            return I2CResultOK;
        }));
    const auto status = miniport.DeployAntenna(&miniport, &i2c, ANTENNA_PRIMARY_CHANNEL, ANTENNA1_ID, TimeSpanFromSeconds(200), true);
    ASSERT_THAT(status, Eq(OSResultSuccess));
}

TEST_F(AntennaMiniportTest, TestManualAntennaDeploymentWithOverrideFailure)
{
    EXPECT_CALL(i2c, I2CWrite(ANTENNA_BACKUP_CHANNEL, DeployAntenna3Override, _, _)).WillOnce(Return(I2CResultNack));
    const auto status = miniport.DeployAntenna(&miniport, &i2c, ANTENNA_BACKUP_CHANNEL, ANTENNA3_ID, TimeSpanFromMilliseconds(200), true);
    ASSERT_THAT(status, Ne(OSResultSuccess));
}

TEST_F(AntennaMiniportTest, TestAntennaActivationCount)
{
    EXPECT_CALL(i2c, I2CWriteRead(ANTENNA_PRIMARY_CHANNEL, QueryActivationCount1, _, _, Ne(nullptr), Ge(1u)))
        .WillOnce(Invoke([=](uint8_t /*address*/,
            uint8_t /*command*/,
            const uint8_t* /*inData*/,
            uint16_t /*length*/,
            uint8_t* outData,
            uint16_t outLength) {
            memset(outData, 0, outLength);
            outData[0] = 10;
            return I2CResultOK;
        }));
    uint16_t response;
    const auto status = miniport.GetAntennaActivationCount(&miniport, &i2c, ANTENNA_PRIMARY_CHANNEL, ANTENNA1_ID, &response);
    ASSERT_THAT(status, Eq(OSResultSuccess));
    ASSERT_THAT(response, Eq(10u));
}

TEST_F(AntennaMiniportTest, TestAntennaActivationCountFailure)
{
    EXPECT_CALL(i2c, I2CWriteRead(ANTENNA_BACKUP_CHANNEL, QueryActivationCount2, _, _, _, _)).WillOnce(Return(I2CResultNack));
    uint16_t response;
    const auto status = miniport.GetAntennaActivationCount(&miniport, &i2c, ANTENNA_BACKUP_CHANNEL, ANTENNA2_ID, &response);
    ASSERT_THAT(status, Ne(OSResultSuccess));
    ASSERT_THAT(response, Eq(0u));
}

TEST_F(AntennaMiniportTest, TestAntennaTemperature)
{
    EXPECT_CALL(i2c, I2CWriteRead(ANTENNA_PRIMARY_CHANNEL, QueryTemperature, _, _, Ne(nullptr), Ge(2u)))
        .WillOnce(Invoke([=](uint8_t /*address*/,
            uint8_t /*command*/,
            const uint8_t* /*inData*/,
            uint16_t /*length*/,
            uint8_t* outData,
            uint16_t outLength) {
            memset(outData, 0, outLength);
            outData[0] = 0;
            outData[1] = 0x11;
            return I2CResultOK;
        }));
    uint16_t response;
    const auto status = miniport.GetTemperature(&miniport, &i2c, ANTENNA_PRIMARY_CHANNEL, &response);
    ASSERT_THAT(status, Eq(OSResultSuccess));
    ASSERT_THAT(response, Eq(0x11));
}

TEST_F(AntennaMiniportTest, TestAntennaTemperatureOutOfRange)
{
    EXPECT_CALL(i2c, I2CWriteRead(ANTENNA_PRIMARY_CHANNEL, QueryTemperature, _, _, Ne(nullptr), Ge(2u)))
        .WillOnce(Invoke([=](uint8_t /*address*/,
            uint8_t /*command*/,
            const uint8_t* /*inData*/,
            uint16_t /*length*/,
            uint8_t* outData,
            uint16_t outLength) {
            memset(outData, 0, outLength);
            outData[0] = 0xfc;
            outData[1] = 0;
            return I2CResultOK;
        }));
    uint16_t response;
    const auto status = miniport.GetTemperature(&miniport, &i2c, ANTENNA_PRIMARY_CHANNEL, &response);
    ASSERT_THAT(status, Eq(OSResultOutOfRange));
    ASSERT_THAT(response, Eq(0));
}

TEST_F(AntennaMiniportTest, TestAntennaTemperatureFailure)
{
    EXPECT_CALL(i2c, I2CWriteRead(ANTENNA_PRIMARY_CHANNEL, QueryTemperature, _, _, _, _)).WillOnce(Return(I2CResultNack));
    uint16_t response;
    const auto status = miniport.GetTemperature(&miniport, &i2c, ANTENNA_PRIMARY_CHANNEL, &response);
    ASSERT_THAT(status, Ne(OSResultSuccess));
    ASSERT_THAT(response, Eq(0u));
}

TEST_F(AntennaMiniportTest, TestAntennaActivationTime)
{
    EXPECT_CALL(i2c, I2CWriteRead(ANTENNA_PRIMARY_CHANNEL, QueryActivationTime1, _, _, Ne(nullptr), Ge(1u)))
        .WillOnce(Invoke([=](uint8_t /*address*/,
            uint8_t /*command*/,
            const uint8_t* /*inData*/,
            uint16_t /*length*/,
            uint8_t* outData,
            uint16_t outLength) {
            memset(outData, 0, outLength);
            outData[0] = 10;
            return I2CResultOK;
        }));
    TimeSpan response;
    const auto status = miniport.GetAntennaActivationTime(&miniport, &i2c, ANTENNA_PRIMARY_CHANNEL, ANTENNA1_ID, &response);
    ASSERT_THAT(status, Eq(OSResultSuccess));
    ASSERT_THAT(response, Eq(TimeSpanFromMilliseconds(128000)));
}

TEST_F(AntennaMiniportTest, TestAntennaActivationTimeFailure)
{
    EXPECT_CALL(i2c, I2CWriteRead(ANTENNA_PRIMARY_CHANNEL, QueryActivationTime2, _, _, _, _)).WillOnce(Return(I2CResultNack));
    TimeSpan response;
    const auto status = miniport.GetAntennaActivationTime(&miniport, &i2c, ANTENNA_PRIMARY_CHANNEL, ANTENNA2_ID, &response);
    ASSERT_THAT(status, Ne(OSResultSuccess));
    ASSERT_THAT(response, Eq(TimeSpanFromMilliseconds(0u)));
}

class AntennaDeploymentStatusTest
    : public testing::TestWithParam<std::tuple<uint8_t, uint8_t, I2CResult, OSResult, uint32_t, uint32_t, bool, bool>>
{
  public:
    AntennaDeploymentStatusTest();
    void MockI2C();

  protected:
    I2CBusMock i2c;
    AntennaMiniportDriver miniport;
};

AntennaDeploymentStatusTest::AntennaDeploymentStatusTest()
{
    AntennaMiniportInitialize(&miniport);
}

void AntennaDeploymentStatusTest::MockI2C()
{
    const auto response1 = std::get<0>(GetParam());
    const auto response2 = std::get<1>(GetParam());
    const auto i2cResult = std::get<2>(GetParam());

    EXPECT_CALL(i2c, I2CWriteRead(ANTENNA_PRIMARY_CHANNEL, QueryDeploymentStatus, _, _, Ne(nullptr), Ge(2u)))
        .WillOnce(Invoke([=](uint8_t /*address*/,
            uint8_t /*command*/,
            const uint8_t* /*inData*/,
            uint16_t /*length*/,
            uint8_t* outData,
            uint16_t outLength) {
            memset(outData, 0, outLength);
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

    const auto result = miniport.GetDeploymentStatus(&miniport, &i2c, ANTENNA_PRIMARY_CHANNEL, &response);
    ASSERT_THAT(result, Eq(expectedResult));
}

TEST_P(AntennaDeploymentStatusTest, TestDeploymentStatusData)
{
    MockI2C();
    AntennaDeploymentStatus response;
    const auto deploymentStatuses = std::get<4>(GetParam());
    const auto deplomentActive = std::get<5>(GetParam());
    const auto overrideActive = std::get<6>(GetParam());
    const auto systemArmed = std::get<7>(GetParam());
    miniport.GetDeploymentStatus(&miniport, &i2c, ANTENNA_PRIMARY_CHANNEL, &response);

    ASSERT_THAT(response.DeploymentStatus[0], (deploymentStatuses & 0xff) != 0);
    ASSERT_THAT(response.DeploymentStatus[1], (deploymentStatuses & 0xff00) != 0);
    ASSERT_THAT(response.DeploymentStatus[2], (deploymentStatuses & 0xff0000) != 0);
    ASSERT_THAT(response.DeploymentStatus[3], (deploymentStatuses & 0xff000000) != 0);

    ASSERT_THAT(response.IsDeploymentActive[0], (deplomentActive & 0xff) != 0);
    ASSERT_THAT(response.IsDeploymentActive[1], (deplomentActive & 0xff00) != 0);
    ASSERT_THAT(response.IsDeploymentActive[2], (deplomentActive & 0xff0000) != 0);
    ASSERT_THAT(response.IsDeploymentActive[3], (deplomentActive & 0xff000000) != 0);

    ASSERT_THAT(response.IgnoringDeploymentSwitches, Eq(overrideActive));
    ASSERT_THAT(response.DeploymentSystemArmed, Eq(systemArmed));
}

INSTANTIATE_TEST_CASE_P(AntennaDeploymentStatusTestSet,
    AntennaDeploymentStatusTest,
    testing::Values(std::make_tuple(0, 0, I2CResultNack, OSResultIOError, 0u, 0u, false, false),
                            std::make_tuple(0, 0, I2CResultOK, OSResultSuccess, 0u, 0u, false, false),
                            std::make_tuple(1, 0, I2CResultOK, OSResultSuccess, 0u, 0u, false, true),
                            std::make_tuple(0, 1, I2CResultOK, OSResultSuccess, 0u, 0u, true, false),
                            std::make_tuple(2, 0, I2CResultOK, OSResultSuccess, 0u, 0x1000000, false, false),
                            std::make_tuple(8, 0, I2CResultOK, OSResultSuccess, 0x1000000, 0u, false, false),
                            std::make_tuple(0x0a, 0, I2CResultOK, OSResultSuccess, 0x1000000, 0x1000000, false, false),
                            std::make_tuple(0x20, 0, I2CResultOK, OSResultSuccess, 0u, 0x10000, false, false),
                            std::make_tuple(0x80, 0, I2CResultOK, OSResultSuccess, 0x10000, 0u, false, false),
                            std::make_tuple(0xa0, 0, I2CResultOK, OSResultSuccess, 0x10000, 0x10000, false, false),
                            std::make_tuple(0, 2, I2CResultOK, OSResultSuccess, 0u, 0x100, false, false),
                            std::make_tuple(0, 8, I2CResultOK, OSResultSuccess, 0x100, 0u, false, false),
                            std::make_tuple(0, 0x0a, I2CResultOK, OSResultSuccess, 0x100, 0x100, false, false),
                            std::make_tuple(0, 0x20, I2CResultOK, OSResultSuccess, 0u, 0x1, false, false),
                            std::make_tuple(0, 0x80, I2CResultOK, OSResultSuccess, 0x1, 0u, false, false),
                            std::make_tuple(0, 0xa0, I2CResultOK, OSResultSuccess, 0x1, 0x1, false, false),
                            std::make_tuple(0, 0x10, I2CResultOK, OSResultOutOfRange, 0u, 0u, false, false)), );
