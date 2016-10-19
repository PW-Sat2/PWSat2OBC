#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "MiniportMock.hpp"
#include "antenna/driver.h"

using testing::Eq;
using testing::Ne;
using testing::_;
using testing::Return;
using testing::Invoke;

class AntennaDriverTest : public testing::Test
{
  protected:
    AntennaDriverTest();

    AntennaDriver driver;
    AntennaMiniportMock primary;
    AntennaMiniportMock secondary;
};

AntennaDriverTest::AntennaDriverTest()
{
    AntennaDriverInitialize(&driver, &primary, &secondary);
}

TEST_F(AntennaDriverTest, TestResetBothChannels)
{
    EXPECT_CALL(primary, Reset()).Times(1);
    EXPECT_CALL(secondary, Reset()).Times(1);
    driver.Reset(&driver);
}

TEST_F(AntennaDriverTest, TestResetStatusSuccess)
{
    EXPECT_CALL(primary, Reset()).WillOnce(Return(OSResultSuccess));
    EXPECT_CALL(secondary, Reset()).WillOnce(Return(OSResultSuccess));
    const auto result = driver.Reset(&driver);
    ASSERT_THAT(result, Eq(OSResultSuccess));
    ASSERT_THAT(driver.primaryChannel.status, Eq(ANTENNA_PORT_OPERATIONAL));
    ASSERT_THAT(driver.secondaryChannel.status, Eq(ANTENNA_PORT_OPERATIONAL));
}

TEST_F(AntennaDriverTest, TestResetPrimaryStatusFailure)
{
    EXPECT_CALL(primary, Reset()).WillOnce(Return(OSResultDeviceNotFound));
    EXPECT_CALL(secondary, Reset()).WillOnce(Return(OSResultSuccess));
    const auto result = driver.Reset(&driver);
    ASSERT_THAT(result, Eq(OSResultSuccess));
    ASSERT_THAT(driver.primaryChannel.status, Eq(ANTENNA_PORT_FAILURE));
}

TEST_F(AntennaDriverTest, TestResetSecondaryStatusFailure)
{
    EXPECT_CALL(primary, Reset()).WillOnce(Return(OSResultSuccess));
    EXPECT_CALL(secondary, Reset()).WillOnce(Return(OSResultDeviceNotFound));
    const auto result = driver.Reset(&driver);
    ASSERT_THAT(result, Eq(OSResultSuccess));
    ASSERT_THAT(driver.secondaryChannel.status, Eq(ANTENNA_PORT_FAILURE));
}

TEST_F(AntennaDriverTest, TestResetBothChannelsFailure)
{
    EXPECT_CALL(primary, Reset()).WillOnce(Return(OSResultIOError));
    EXPECT_CALL(secondary, Reset()).WillOnce(Return(OSResultDeviceNotFound));
    const auto result = driver.Reset(&driver);
    ASSERT_THAT(result, Ne(OSResultSuccess));
}

TEST_F(AntennaDriverTest, TestGetTemperaturePrimaryChannel)
{
    EXPECT_CALL(primary, GetTemperature(_))
        .WillOnce(Invoke([](uint16_t* value) //
            {
                *value = 10;
                return OSResultSuccess;
            }));

    EXPECT_CALL(secondary, GetTemperature(_)).WillOnce(Return(OSResultDeviceNotFound));
    uint16_t result;
    const auto status = driver.GetTemperature(&driver, &result);
    ASSERT_THAT(status, Eq(OSResultSuccess));
    ASSERT_THAT(result, Eq(10));
}

TEST_F(AntennaDriverTest, TestGetTemperatureSecondaryChannel)
{
    EXPECT_CALL(secondary, GetTemperature(_))
        .WillOnce(Invoke([](uint16_t* value) //
            {
                *value = 20;
                return OSResultSuccess;
            }));

    EXPECT_CALL(primary, GetTemperature(_)).WillOnce(Return(OSResultDeviceNotFound));
    uint16_t result;
    const auto status = driver.GetTemperature(&driver, &result);
    ASSERT_THAT(status, Eq(OSResultSuccess));
    ASSERT_THAT(result, Eq(20));
}

TEST_F(AntennaDriverTest, TestGetTelemetryPositiveTestSecondaryChannel)
{
    EXPECT_CALL(secondary, GetDeploymentStatus(_)).WillOnce(Return(OSResultSuccess));
    EXPECT_CALL(secondary, GetTemperature(_)).WillOnce(Return(OSResultSuccess));
    EXPECT_CALL(secondary, GetAntennaActivationCount(_, _)).Times(4).WillRepeatedly(Return(OSResultSuccess));
    EXPECT_CALL(secondary, GetAntennaActivationTime(_, _)).Times(4).WillRepeatedly(Return(OSResultSuccess));

    EXPECT_CALL(primary, GetDeploymentStatus(_)).WillOnce(Return(OSResultIOError));

    const auto result = driver.GetTelemetry(&driver);
    ASSERT_THAT(result.flags,
        Eq(static_cast<uint32_t>(ANT_TM_ANTENNA_DEPLOYMENT_STATUS | //
            ANT_TM_ANTENNA_DEPLOYMENT_ACTIVE |                      //
            ANT_TM_ANTENNA1_ACTIVATION_COUNT |                      //
            ANT_TM_ANTENNA2_ACTIVATION_COUNT |                      //
            ANT_TM_ANTENNA3_ACTIVATION_COUNT |                      //
            ANT_TM_ANTENNA4_ACTIVATION_COUNT |                      //
            ANT_TM_ANTENNA1_ACTIVATION_TIME |                       //
            ANT_TM_ANTENNA2_ACTIVATION_TIME |                       //
            ANT_TM_ANTENNA3_ACTIVATION_TIME |                       //
            ANT_TM_ANTENNA4_ACTIVATION_TIME |                       //
            ANT_TM_TEMPERATURE2 |                                   //
            ANT_TM_SWITCHES_IGNORED2 |                              //
            ANT_TM_DEPLOYMENT_SYSTEM_STATUS2                        //
            )));
}

TEST_F(AntennaDriverTest, TestGetTelemetryPositiveTestPrimaryChannel)
{
    EXPECT_CALL(primary, GetDeploymentStatus(_)).WillOnce(Return(OSResultSuccess));
    EXPECT_CALL(primary, GetTemperature(_)).WillOnce(Return(OSResultSuccess));
    EXPECT_CALL(primary, GetAntennaActivationCount(_, _)).Times(4).WillRepeatedly(Return(OSResultSuccess));
    EXPECT_CALL(primary, GetAntennaActivationTime(_, _)).Times(4).WillRepeatedly(Return(OSResultSuccess));

    EXPECT_CALL(secondary, GetDeploymentStatus(_)).WillOnce(Return(OSResultIOError));

    const auto result = driver.GetTelemetry(&driver);
    ASSERT_THAT(result.flags,
        Eq(static_cast<uint32_t>(ANT_TM_ANTENNA_DEPLOYMENT_STATUS | //
            ANT_TM_ANTENNA_DEPLOYMENT_ACTIVE |                      //
            ANT_TM_ANTENNA1_ACTIVATION_COUNT |                      //
            ANT_TM_ANTENNA2_ACTIVATION_COUNT |                      //
            ANT_TM_ANTENNA3_ACTIVATION_COUNT |                      //
            ANT_TM_ANTENNA4_ACTIVATION_COUNT |                      //
            ANT_TM_ANTENNA1_ACTIVATION_TIME |                       //
            ANT_TM_ANTENNA2_ACTIVATION_TIME |                       //
            ANT_TM_ANTENNA3_ACTIVATION_TIME |                       //
            ANT_TM_ANTENNA4_ACTIVATION_TIME |                       //
            ANT_TM_TEMPERATURE1 |                                   //
            ANT_TM_SWITCHES_IGNORED1 |                              //
            ANT_TM_DEPLOYMENT_SYSTEM_STATUS1                        //
            )));
}

TEST_F(AntennaDriverTest, TestGetTelemetryBothChannelsFailure)
{
    EXPECT_CALL(primary, GetDeploymentStatus(_)).WillOnce(Return(OSResultIOError));
    EXPECT_CALL(secondary, GetDeploymentStatus(_)).WillOnce(Return(OSResultIOError));

    const auto result = driver.GetTelemetry(&driver);
    ASSERT_THAT(result.flags, Eq(0u));
}

class AntennaChannelStateManagementTest
    : public testing::TestWithParam<
          std::tuple<OSResult, OSResult, OSResult, AntenaPortStatus, AntenaPortStatus, AntenaPortStatus, AntenaPortStatus>>
{
  public:
    AntennaChannelStateManagementTest();

  protected:
    bool PrimaryChannelActive() const;
    bool SecondaryChannelActive() const;
    AntennaDriver driver;
    AntennaMiniportMock primary;
    AntennaMiniportMock secondary;
};

AntennaChannelStateManagementTest::AntennaChannelStateManagementTest()
{
    AntennaDriverInitialize(&driver, &primary, &secondary);
    driver.primaryChannel.status = std::get<3>(GetParam());
    driver.secondaryChannel.status = std::get<4>(GetParam());
}

bool AntennaChannelStateManagementTest::PrimaryChannelActive() const
{
    return driver.primaryChannel.status == ANTENNA_PORT_OPERATIONAL;
}

bool AntennaChannelStateManagementTest::SecondaryChannelActive() const
{
    return driver.secondaryChannel.status == ANTENNA_PORT_OPERATIONAL;
}

TEST_P(AntennaChannelStateManagementTest, TestArmingDeploymentSystem)
{
    if (PrimaryChannelActive())
    {
        EXPECT_CALL(primary, ArmDeploymentSystem()).WillOnce(Return(std::get<0>(GetParam())));
    }

    if (SecondaryChannelActive())
    {
        EXPECT_CALL(secondary, ArmDeploymentSystem()).WillOnce(Return(std::get<1>(GetParam())));
    }

    const auto status = driver.ArmDeploymentSystem(&driver);
    ASSERT_THAT(status, Eq(std::get<2>(GetParam())));
    ASSERT_THAT(driver.primaryChannel.status, Eq(std::get<5>(GetParam())));
    ASSERT_THAT(driver.secondaryChannel.status, Eq(std::get<6>(GetParam())));
}

TEST_P(AntennaChannelStateManagementTest, TestDisarmingDeploymentSystem)
{
    if (PrimaryChannelActive())
    {
        EXPECT_CALL(primary, DisarmDeploymentSystem()).WillOnce(Return(std::get<0>(GetParam())));
    }

    if (SecondaryChannelActive())
    {
        EXPECT_CALL(secondary, DisarmDeploymentSystem()).WillOnce(Return(std::get<1>(GetParam())));
    }

    const auto status = driver.DisarmDeploymentSystem(&driver);
    ASSERT_THAT(status, Eq(std::get<2>(GetParam())));
    ASSERT_THAT(driver.primaryChannel.status, Eq(std::get<5>(GetParam())));
    ASSERT_THAT(driver.secondaryChannel.status, Eq(std::get<6>(GetParam())));
}

TEST_P(AntennaChannelStateManagementTest, TestAntennaDeployment)
{
    if (PrimaryChannelActive())
    {
        EXPECT_CALL(primary, DeployAntenna(ANTENNA2, _)).WillOnce(Return(std::get<0>(GetParam())));
    }

    if (SecondaryChannelActive())
    {
        EXPECT_CALL(secondary, DeployAntenna(ANTENNA2, _)).WillOnce(Return(std::get<1>(GetParam())));
    }

    const auto status = driver.DeployAntenna(&driver, ANTENNA2, TimeSpanFromMilliseconds(1));
    ASSERT_THAT(status, Eq(std::get<2>(GetParam())));
    ASSERT_THAT(driver.primaryChannel.status, Eq(std::get<5>(GetParam())));
    ASSERT_THAT(driver.secondaryChannel.status, Eq(std::get<6>(GetParam())));
}

TEST_P(AntennaChannelStateManagementTest, TestAntennaDeploymentOverride)
{
    if (PrimaryChannelActive())
    {
        EXPECT_CALL(primary, DeployAntennaOverride(ANTENNA2, _)).WillOnce(Return(std::get<0>(GetParam())));
    }

    if (SecondaryChannelActive())
    {
        EXPECT_CALL(secondary, DeployAntennaOverride(ANTENNA2, _)).WillOnce(Return(std::get<1>(GetParam())));
    }

    const auto status = driver.DeployAntennaOverride(&driver, ANTENNA2, TimeSpanFromMilliseconds(1));
    ASSERT_THAT(status, Eq(std::get<2>(GetParam())));
    ASSERT_THAT(driver.primaryChannel.status, Eq(std::get<5>(GetParam())));
    ASSERT_THAT(driver.secondaryChannel.status, Eq(std::get<6>(GetParam())));
}

TEST_P(AntennaChannelStateManagementTest, TestAutomaticDeployment)
{
    if (PrimaryChannelActive())
    {
        EXPECT_CALL(primary, InitializeAutomaticDeployment()).WillOnce(Return(std::get<0>(GetParam())));
    }

    if (SecondaryChannelActive())
    {
        EXPECT_CALL(secondary, InitializeAutomaticDeployment()).WillOnce(Return(std::get<1>(GetParam())));
    }

    const auto status = driver.InitializeAutomaticDeployment(&driver);
    ASSERT_THAT(status, Eq(std::get<2>(GetParam())));
    ASSERT_THAT(driver.primaryChannel.status, Eq(std::get<5>(GetParam())));
    ASSERT_THAT(driver.secondaryChannel.status, Eq(std::get<6>(GetParam())));
}

TEST_P(AntennaChannelStateManagementTest, TestCancelAutomaticDeployment)
{
    if (PrimaryChannelActive())
    {
        EXPECT_CALL(primary, CancelAntennaDeployment()).WillOnce(Return(std::get<0>(GetParam())));
    }

    if (SecondaryChannelActive())
    {
        EXPECT_CALL(secondary, CancelAntennaDeployment()).WillOnce(Return(std::get<1>(GetParam())));
    }

    const auto status = driver.CancelAntennaDeployment(&driver);
    ASSERT_THAT(status, Eq(std::get<2>(GetParam())));
    ASSERT_THAT(driver.primaryChannel.status, Eq(std::get<5>(GetParam())));
    ASSERT_THAT(driver.secondaryChannel.status, Eq(std::get<6>(GetParam())));
}

TEST_P(AntennaChannelStateManagementTest, TestGetTemperature)
{
    if (PrimaryChannelActive())
    {
        EXPECT_CALL(primary, GetTemperature(_)).WillOnce(Return(std::get<0>(GetParam())));
    }

    if (SecondaryChannelActive())
    {
        EXPECT_CALL(secondary, GetTemperature(_)).WillOnce(Return(std::get<1>(GetParam())));
    }

    uint16_t result;
    const auto status = driver.GetTemperature(&driver, &result);
    ASSERT_THAT(status, Eq(std::get<2>(GetParam())));
    ASSERT_THAT(driver.primaryChannel.status, Eq(std::get<5>(GetParam())));
    ASSERT_THAT(driver.secondaryChannel.status, Eq(std::get<6>(GetParam())));
}

INSTANTIATE_TEST_CASE_P(AntennaChannelTestSet,
    AntennaChannelStateManagementTest,
    testing::Values( //
                            std::make_tuple(OSResultSuccess,
                                OSResultSuccess,
                                OSResultSuccess,
                                ANTENNA_PORT_OPERATIONAL,
                                ANTENNA_PORT_OPERATIONAL,
                                ANTENNA_PORT_OPERATIONAL,
                                ANTENNA_PORT_OPERATIONAL),

                            std::make_tuple(OSResultIOError,
                                OSResultIOError,
                                OSResultIOError,
                                ANTENNA_PORT_OPERATIONAL,
                                ANTENNA_PORT_OPERATIONAL,
                                ANTENNA_PORT_FAILURE,
                                ANTENNA_PORT_FAILURE),

                            std::make_tuple(OSResultSuccess,
                                OSResultIOError,
                                OSResultSuccess,
                                ANTENNA_PORT_OPERATIONAL,
                                ANTENNA_PORT_OPERATIONAL,
                                ANTENNA_PORT_OPERATIONAL,
                                ANTENNA_PORT_FAILURE),

                            std::make_tuple(OSResultIOError,
                                OSResultSuccess,
                                OSResultSuccess,
                                ANTENNA_PORT_OPERATIONAL,
                                ANTENNA_PORT_OPERATIONAL,
                                ANTENNA_PORT_FAILURE,
                                ANTENNA_PORT_OPERATIONAL),

                            std::make_tuple(OSResultSuccess,
                                OSResultSuccess,
                                OSResultIOError,
                                ANTENNA_PORT_FAILURE,
                                ANTENNA_PORT_FAILURE,
                                ANTENNA_PORT_FAILURE,
                                ANTENNA_PORT_FAILURE),

                            std::make_tuple(OSResultSuccess,
                                OSResultSuccess,
                                OSResultSuccess,
                                ANTENNA_PORT_OPERATIONAL,
                                ANTENNA_PORT_FAILURE,
                                ANTENNA_PORT_OPERATIONAL,
                                ANTENNA_PORT_FAILURE),

                            std::make_tuple(OSResultSuccess,
                                OSResultSuccess,
                                OSResultSuccess,
                                ANTENNA_PORT_FAILURE,
                                ANTENNA_PORT_OPERATIONAL,
                                ANTENNA_PORT_FAILURE,
                                ANTENNA_PORT_OPERATIONAL),

                            std::make_tuple(OSResultSuccess,
                                OSResultIOError,
                                OSResultIOError,
                                ANTENNA_PORT_FAILURE,
                                ANTENNA_PORT_OPERATIONAL,
                                ANTENNA_PORT_FAILURE,
                                ANTENNA_PORT_FAILURE)), );
