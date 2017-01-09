#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "I2C/I2CMock.hpp"
#include "MiniportMock.hpp"
#include "antenna/driver.h"

using testing::Eq;
using testing::Ne;
using testing::_;
using testing::Return;
using testing::Invoke;
using std::chrono::seconds;

class AntennaDriverTest : public testing::Test
{
  protected:
    AntennaDriverTest();

    AntennaDriver driver;
    AntennaMiniportMock primary;
    I2CBusMock i2c;
};

AntennaDriverTest::AntennaDriverTest()
{
    AntennaDriverInitialize(&driver, &primary, &i2c, &i2c);
}

TEST_F(AntennaDriverTest, TestHardResetBothChannels)
{
    EXPECT_CALL(primary, Reset(ANTENNA_PRIMARY_CHANNEL)).Times(1);
    EXPECT_CALL(primary, Reset(ANTENNA_BACKUP_CHANNEL)).Times(1);
    driver.HardReset(&driver);
}

TEST_F(AntennaDriverTest, TestHardResetStatusSuccess)
{
    EXPECT_CALL(primary, Reset(ANTENNA_PRIMARY_CHANNEL)).WillOnce(Return(OSResult::Success));
    EXPECT_CALL(primary, Reset(ANTENNA_BACKUP_CHANNEL)).WillOnce(Return(OSResult::Success));
    const auto result = driver.HardReset(&driver);
    ASSERT_THAT(result, Eq(OSResult::Success));
    ASSERT_THAT(driver.primaryChannel.status, Eq(ANTENNA_PORT_OPERATIONAL));
    ASSERT_THAT(driver.secondaryChannel.status, Eq(ANTENNA_PORT_OPERATIONAL));
}

TEST_F(AntennaDriverTest, TestHardResetPrimaryStatusFailure)
{
    EXPECT_CALL(primary, Reset(ANTENNA_PRIMARY_CHANNEL)).WillOnce(Return(OSResult::DeviceNotFound));
    EXPECT_CALL(primary, Reset(ANTENNA_BACKUP_CHANNEL)).WillOnce(Return(OSResult::Success));
    const auto result = driver.HardReset(&driver);
    ASSERT_THAT(result, Eq(OSResult::Success));
    ASSERT_THAT(driver.primaryChannel.status, Eq(ANTENNA_PORT_FAILURE));
}

TEST_F(AntennaDriverTest, TestHardResetSecondaryStatusFailure)
{
    EXPECT_CALL(primary, Reset(ANTENNA_PRIMARY_CHANNEL)).WillOnce(Return(OSResult::Success));
    EXPECT_CALL(primary, Reset(ANTENNA_BACKUP_CHANNEL)).WillOnce(Return(OSResult::DeviceNotFound));
    const auto result = driver.HardReset(&driver);
    ASSERT_THAT(result, Eq(OSResult::Success));
    ASSERT_THAT(driver.secondaryChannel.status, Eq(ANTENNA_PORT_FAILURE));
}

TEST_F(AntennaDriverTest, TestResetStatusSuccess)
{
    driver.primaryChannel.status = ANTENNA_PORT_FAILURE;
    EXPECT_CALL(primary, Reset(ANTENNA_PRIMARY_CHANNEL)).WillOnce(Return(OSResult::Success));
    const auto result = driver.Reset(&driver, ANTENNA_PRIMARY_CHANNEL);
    ASSERT_THAT(result, Eq(OSResult::Success));
    ASSERT_THAT(driver.primaryChannel.status, Eq(ANTENNA_PORT_OPERATIONAL));
}

TEST_F(AntennaDriverTest, TestResetStatusFailure)
{
    EXPECT_CALL(primary, Reset(ANTENNA_BACKUP_CHANNEL)).WillOnce(Return(OSResult::DeviceNotFound));
    const auto result = driver.Reset(&driver, ANTENNA_BACKUP_CHANNEL);
    ASSERT_THAT(result, Ne(OSResult::Success));
    ASSERT_THAT(driver.secondaryChannel.status, Eq(ANTENNA_PORT_FAILURE));
}

TEST_F(AntennaDriverTest, TestHardResetBothChannelsFailure)
{
    EXPECT_CALL(primary, Reset(ANTENNA_PRIMARY_CHANNEL)).WillOnce(Return(OSResult::DeviceNotFound));
    EXPECT_CALL(primary, Reset(ANTENNA_BACKUP_CHANNEL)).WillOnce(Return(OSResult::DeviceNotFound));
    const auto result = driver.HardReset(&driver);
    ASSERT_THAT(result, Ne(OSResult::Success));
}

TEST_F(AntennaDriverTest, TestGetTemperature)
{
    EXPECT_CALL(primary, GetTemperature(ANTENNA_PRIMARY_CHANNEL, _))
        .WillOnce(Invoke([](AntennaChannel channel, uint16_t* value) //
            {
                UNREFERENCED_PARAMETER(channel);
                *value = 10;
                return OSResult::Success;
            }));

    uint16_t result;
    const auto status = driver.GetTemperature(&driver, ANTENNA_PRIMARY_CHANNEL, &result);
    ASSERT_THAT(status, Eq(OSResult::Success));
    ASSERT_THAT(result, Eq(10));
}

TEST_F(AntennaDriverTest, TestGetTemperatureFailure)
{
    EXPECT_CALL(primary, GetTemperature(ANTENNA_BACKUP_CHANNEL, _)).WillOnce(Return(OSResult::IOError));
    uint16_t result;
    const auto status = driver.GetTemperature(&driver, ANTENNA_BACKUP_CHANNEL, &result);
    ASSERT_THAT(status, Ne(OSResult::Success));
}

TEST_F(AntennaDriverTest, TestGetTelemetryPositiveTestSecondaryChannel)
{
    EXPECT_CALL(primary, GetTemperature(ANTENNA_BACKUP_CHANNEL, _)).WillOnce(Return(OSResult::Success));
    EXPECT_CALL(primary, GetAntennaActivationCount(ANTENNA_BACKUP_CHANNEL, _, _)).Times(4).WillRepeatedly(Return(OSResult::Success));
    EXPECT_CALL(primary, GetAntennaActivationTime(ANTENNA_BACKUP_CHANNEL, _, _)).Times(4).WillRepeatedly(Return(OSResult::Success));

    EXPECT_CALL(primary, GetTemperature(ANTENNA_PRIMARY_CHANNEL, _)).WillOnce(Return(OSResult::DeviceNotFound));
    EXPECT_CALL(primary, GetAntennaActivationCount(ANTENNA_PRIMARY_CHANNEL, _, _))
        .Times(4)
        .WillRepeatedly(Return(OSResult::DeviceNotFound));
    EXPECT_CALL(primary, GetAntennaActivationTime(ANTENNA_PRIMARY_CHANNEL, _, _)).Times(4).WillRepeatedly(Return(OSResult::DeviceNotFound));

    const auto result = driver.GetTelemetry(&driver);
    ASSERT_THAT(result.flags,
        Eq(static_cast<uint32_t>(ANT_TM_ANTENNA1_ACTIVATION_COUNT | //
            ANT_TM_ANTENNA2_ACTIVATION_COUNT |                      //
            ANT_TM_ANTENNA3_ACTIVATION_COUNT |                      //
            ANT_TM_ANTENNA4_ACTIVATION_COUNT |                      //
            ANT_TM_ANTENNA1_ACTIVATION_TIME |                       //
            ANT_TM_ANTENNA2_ACTIVATION_TIME |                       //
            ANT_TM_ANTENNA3_ACTIVATION_TIME |                       //
            ANT_TM_ANTENNA4_ACTIVATION_TIME |                       //
            ANT_TM_TEMPERATURE2                                     //
            )));
}

TEST_F(AntennaDriverTest, TestGetTelemetryPositiveTestPrimaryChannel)
{
    EXPECT_CALL(primary, GetTemperature(ANTENNA_PRIMARY_CHANNEL, _)).WillOnce(Return(OSResult::Success));
    EXPECT_CALL(primary, GetAntennaActivationCount(ANTENNA_PRIMARY_CHANNEL, _, _)).Times(4).WillRepeatedly(Return(OSResult::Success));
    EXPECT_CALL(primary, GetAntennaActivationTime(ANTENNA_PRIMARY_CHANNEL, _, _)).Times(4).WillRepeatedly(Return(OSResult::Success));

    EXPECT_CALL(primary, GetTemperature(ANTENNA_BACKUP_CHANNEL, _)).WillOnce(Return(OSResult::DeviceNotFound));
    EXPECT_CALL(primary, GetAntennaActivationCount(ANTENNA_BACKUP_CHANNEL, _, _)).Times(4).WillRepeatedly(Return(OSResult::DeviceNotFound));
    EXPECT_CALL(primary, GetAntennaActivationTime(ANTENNA_BACKUP_CHANNEL, _, _)).Times(4).WillRepeatedly(Return(OSResult::DeviceNotFound));

    const auto result = driver.GetTelemetry(&driver);
    ASSERT_THAT(result.flags,
        Eq(static_cast<uint32_t>(ANT_TM_ANTENNA1_ACTIVATION_COUNT | //
            ANT_TM_ANTENNA2_ACTIVATION_COUNT |                      //
            ANT_TM_ANTENNA3_ACTIVATION_COUNT |                      //
            ANT_TM_ANTENNA4_ACTIVATION_COUNT |                      //
            ANT_TM_ANTENNA1_ACTIVATION_TIME |                       //
            ANT_TM_ANTENNA2_ACTIVATION_TIME |                       //
            ANT_TM_ANTENNA3_ACTIVATION_TIME |                       //
            ANT_TM_ANTENNA4_ACTIVATION_TIME |                       //
            ANT_TM_TEMPERATURE1                                     //
            )));
}

TEST_F(AntennaDriverTest, TestGetTelemetryBothChannelsFailure)
{
    EXPECT_CALL(primary, GetTemperature(_, _)).WillRepeatedly(Return(OSResult::DeviceNotFound));
    EXPECT_CALL(primary, GetAntennaActivationCount(_, _, _)).WillRepeatedly(Return(OSResult::DeviceNotFound));
    EXPECT_CALL(primary, GetAntennaActivationTime(_, _, _)).WillRepeatedly(Return(OSResult::DeviceNotFound));

    const auto result = driver.GetTelemetry(&driver);
    ASSERT_THAT(result.flags, Eq(0u));
}

TEST_F(AntennaDriverTest, TestAutoDeployAntennaArmsDeploymentSystem)
{
    EXPECT_CALL(primary, ArmDeploymentSystem(ANTENNA_PRIMARY_CHANNEL)).Times(1);
    driver.DeployAntenna(&driver, ANTENNA_PRIMARY_CHANNEL, ANTENNA_AUTO_ID, seconds(10), false);
}

TEST_F(AntennaDriverTest, TestManualDeployAntennaArmsDeploymentSystem)
{
    EXPECT_CALL(primary, ArmDeploymentSystem(ANTENNA_PRIMARY_CHANNEL)).Times(1);
    driver.DeployAntenna(&driver, ANTENNA_PRIMARY_CHANNEL, ANTENNA3_ID, seconds(10), false);
}

TEST_F(AntennaDriverTest, TestDeployAntennaArmingFailure)
{
    EXPECT_CALL(primary, ArmDeploymentSystem(ANTENNA_PRIMARY_CHANNEL)).WillOnce(Return(OSResult::DeviceNotFound));
    const auto status = driver.DeployAntenna(&driver, ANTENNA_PRIMARY_CHANNEL, ANTENNA3_ID, seconds(10), false);
    ASSERT_THAT(status, Ne(OSResult::Success));
}

TEST_F(AntennaDriverTest, TestAutomaticDeploymentSuccess)
{
    ON_CALL(primary, ArmDeploymentSystem(_)).WillByDefault(Return(OSResult::Success));
    EXPECT_CALL(primary, InitializeAutomaticDeployment(ANTENNA_BACKUP_CHANNEL, _)).WillOnce(Return(OSResult::Success));
    const auto status = driver.DeployAntenna(&driver, ANTENNA_BACKUP_CHANNEL, ANTENNA_AUTO_ID, seconds(20), false);
    ASSERT_THAT(status, Eq(OSResult::Success));
}

TEST_F(AntennaDriverTest, TestAutomaticDeploymentFailure)
{
    ON_CALL(primary, ArmDeploymentSystem(_)).WillByDefault(Return(OSResult::Success));
    EXPECT_CALL(primary, InitializeAutomaticDeployment(ANTENNA_BACKUP_CHANNEL, _)).WillOnce(Return(OSResult::DeviceNotFound));
    const auto status = driver.DeployAntenna(&driver, ANTENNA_BACKUP_CHANNEL, ANTENNA_AUTO_ID, seconds(20), false);
    ASSERT_THAT(status, Ne(OSResult::Success));
}

TEST_F(AntennaDriverTest, TestManualDeploymentSuccess)
{
    ON_CALL(primary, ArmDeploymentSystem(_)).WillByDefault(Return(OSResult::Success));
    EXPECT_CALL(primary, DeployAntenna(ANTENNA_BACKUP_CHANNEL, ANTENNA3_ID, _, false)).WillOnce(Return(OSResult::Success));
    const auto status = driver.DeployAntenna(&driver, ANTENNA_BACKUP_CHANNEL, ANTENNA3_ID, seconds(20), false);
    ASSERT_THAT(status, Eq(OSResult::Success));
}

TEST_F(AntennaDriverTest, TestManualDeploymentFailure)
{
    ON_CALL(primary, ArmDeploymentSystem(_)).WillByDefault(Return(OSResult::Success));
    EXPECT_CALL(primary, DeployAntenna(ANTENNA_BACKUP_CHANNEL, ANTENNA3_ID, _, false)).WillOnce(Return(OSResult::DeviceNotFound));
    const auto status = driver.DeployAntenna(&driver, ANTENNA_BACKUP_CHANNEL, ANTENNA3_ID, seconds(20), false);
    ASSERT_THAT(status, Ne(OSResult::Success));
}

TEST_F(AntennaDriverTest, TestManualDeploymentSuccessWithOverride)
{
    ON_CALL(primary, ArmDeploymentSystem(_)).WillByDefault(Return(OSResult::Success));
    EXPECT_CALL(primary, DeployAntenna(ANTENNA_BACKUP_CHANNEL, ANTENNA3_ID, _, true)).WillOnce(Return(OSResult::Success));
    const auto status = driver.DeployAntenna(&driver, ANTENNA_BACKUP_CHANNEL, ANTENNA3_ID, seconds(20), true);
    ASSERT_THAT(status, Eq(OSResult::Success));
}

TEST_F(AntennaDriverTest, TestFinishDeploymentCancelsDeployment)
{
    EXPECT_CALL(primary, CancelAntennaDeployment(ANTENNA_PRIMARY_CHANNEL)).Times(1);
    driver.FinishDeployment(&driver, ANTENNA_PRIMARY_CHANNEL);
}

TEST_F(AntennaDriverTest, TestFinishDeploymentDisarmsDeploymentSystem)
{
    ON_CALL(primary, CancelAntennaDeployment(_)).WillByDefault(Return(OSResult::Success));
    EXPECT_CALL(primary, DisarmDeploymentSystem(ANTENNA_PRIMARY_CHANNEL)).WillOnce(Return(OSResult::Success));
    const auto result = driver.FinishDeployment(&driver, ANTENNA_PRIMARY_CHANNEL);
    ASSERT_THAT(result, Eq(OSResult::Success));
}

TEST_F(AntennaDriverTest, TestFinishDeploymentCancelFailure)
{
    ON_CALL(primary, CancelAntennaDeployment(_)).WillByDefault(Return(OSResult::DeviceNotFound));
    EXPECT_CALL(primary, DisarmDeploymentSystem(ANTENNA_PRIMARY_CHANNEL)).Times(0);
    const auto result = driver.FinishDeployment(&driver, ANTENNA_PRIMARY_CHANNEL);
    ASSERT_THAT(result, Ne(OSResult::Success));
}

TEST_F(AntennaDriverTest, TestFinishDeploymentDisarmingFailure)
{
    ON_CALL(primary, CancelAntennaDeployment(_)).WillByDefault(Return(OSResult::Success));
    ON_CALL(primary, DisarmDeploymentSystem(_)).WillByDefault(Return(OSResult::DeviceNotFound));
    const auto result = driver.FinishDeployment(&driver, ANTENNA_PRIMARY_CHANNEL);
    ASSERT_THAT(result, Ne(OSResult::Success));
}
