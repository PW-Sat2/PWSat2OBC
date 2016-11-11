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
    EXPECT_CALL(primary, Reset(ANTENNA_PRIMARY_CHANNEL)).WillOnce(Return(OSResultSuccess));
    EXPECT_CALL(primary, Reset(ANTENNA_BACKUP_CHANNEL)).WillOnce(Return(OSResultSuccess));
    const auto result = driver.HardReset(&driver);
    ASSERT_THAT(result, Eq(OSResultSuccess));
    ASSERT_THAT(driver.primaryChannel.status, Eq(ANTENNA_PORT_OPERATIONAL));
    ASSERT_THAT(driver.secondaryChannel.status, Eq(ANTENNA_PORT_OPERATIONAL));
}

TEST_F(AntennaDriverTest, TestHardResetPrimaryStatusFailure)
{
    EXPECT_CALL(primary, Reset(ANTENNA_PRIMARY_CHANNEL)).WillOnce(Return(OSResultDeviceNotFound));
    EXPECT_CALL(primary, Reset(ANTENNA_BACKUP_CHANNEL)).WillOnce(Return(OSResultSuccess));
    const auto result = driver.HardReset(&driver);
    ASSERT_THAT(result, Eq(OSResultSuccess));
    ASSERT_THAT(driver.primaryChannel.status, Eq(ANTENNA_PORT_FAILURE));
}

TEST_F(AntennaDriverTest, TestHardResetSecondaryStatusFailure)
{
    EXPECT_CALL(primary, Reset(ANTENNA_PRIMARY_CHANNEL)).WillOnce(Return(OSResultSuccess));
    EXPECT_CALL(primary, Reset(ANTENNA_BACKUP_CHANNEL)).WillOnce(Return(OSResultDeviceNotFound));
    const auto result = driver.HardReset(&driver);
    ASSERT_THAT(result, Eq(OSResultSuccess));
    ASSERT_THAT(driver.secondaryChannel.status, Eq(ANTENNA_PORT_FAILURE));
}

TEST_F(AntennaDriverTest, TestResetStatusSuccess)
{
    driver.primaryChannel.status = ANTENNA_PORT_FAILURE;
    EXPECT_CALL(primary, Reset(ANTENNA_PRIMARY_CHANNEL)).WillOnce(Return(OSResultSuccess));
    const auto result = driver.Reset(&driver, ANTENNA_PRIMARY_CHANNEL);
    ASSERT_THAT(result, Eq(OSResultSuccess));
    ASSERT_THAT(driver.primaryChannel.status, Eq(ANTENNA_PORT_OPERATIONAL));
}

TEST_F(AntennaDriverTest, TestResetStatusFailure)
{
    EXPECT_CALL(primary, Reset(ANTENNA_BACKUP_CHANNEL)).WillOnce(Return(OSResultDeviceNotFound));
    const auto result = driver.Reset(&driver, ANTENNA_BACKUP_CHANNEL);
    ASSERT_THAT(result, Ne(OSResultSuccess));
    ASSERT_THAT(driver.secondaryChannel.status, Eq(ANTENNA_PORT_FAILURE));
}

TEST_F(AntennaDriverTest, TestHardResetBothChannelsFailure)
{
    EXPECT_CALL(primary, Reset(ANTENNA_PRIMARY_CHANNEL)).WillOnce(Return(OSResultDeviceNotFound));
    EXPECT_CALL(primary, Reset(ANTENNA_BACKUP_CHANNEL)).WillOnce(Return(OSResultDeviceNotFound));
    const auto result = driver.HardReset(&driver);
    ASSERT_THAT(result, Ne(OSResultSuccess));
}

TEST_F(AntennaDriverTest, TestGetTemperature)
{
    EXPECT_CALL(primary, GetTemperature(ANTENNA_PRIMARY_CHANNEL, _))
        .WillOnce(Invoke([](AntennaChannel channel, uint16_t* value) //
            {
                UNREFERENCED_PARAMETER(channel);
                *value = 10;
                return OSResultSuccess;
            }));

    uint16_t result;
    const auto status = driver.GetTemperature(&driver, ANTENNA_PRIMARY_CHANNEL, &result);
    ASSERT_THAT(status, Eq(OSResultSuccess));
    ASSERT_THAT(result, Eq(10));
}

TEST_F(AntennaDriverTest, TestGetTemperatureFailure)
{
    EXPECT_CALL(primary, GetTemperature(ANTENNA_BACKUP_CHANNEL, _)).WillOnce(Return(OSResultIOError));
    uint16_t result;
    const auto status = driver.GetTemperature(&driver, ANTENNA_BACKUP_CHANNEL, &result);
    ASSERT_THAT(status, Ne(OSResultSuccess));
}

TEST_F(AntennaDriverTest, TestGetTelemetryPositiveTestSecondaryChannel)
{
    EXPECT_CALL(primary, GetTemperature(ANTENNA_BACKUP_CHANNEL, _)).WillOnce(Return(OSResultSuccess));
    EXPECT_CALL(primary, GetAntennaActivationCount(ANTENNA_BACKUP_CHANNEL, _, _)).Times(4).WillRepeatedly(Return(OSResultSuccess));
    EXPECT_CALL(primary, GetAntennaActivationTime(ANTENNA_BACKUP_CHANNEL, _, _)).Times(4).WillRepeatedly(Return(OSResultSuccess));

    EXPECT_CALL(primary, GetTemperature(ANTENNA_PRIMARY_CHANNEL, _)).WillOnce(Return(OSResultDeviceNotFound));
    EXPECT_CALL(primary, GetAntennaActivationCount(ANTENNA_PRIMARY_CHANNEL, _, _)).Times(4).WillRepeatedly(Return(OSResultDeviceNotFound));
    EXPECT_CALL(primary, GetAntennaActivationTime(ANTENNA_PRIMARY_CHANNEL, _, _)).Times(4).WillRepeatedly(Return(OSResultDeviceNotFound));

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
    EXPECT_CALL(primary, GetTemperature(ANTENNA_PRIMARY_CHANNEL, _)).WillOnce(Return(OSResultSuccess));
    EXPECT_CALL(primary, GetAntennaActivationCount(ANTENNA_PRIMARY_CHANNEL, _, _)).Times(4).WillRepeatedly(Return(OSResultSuccess));
    EXPECT_CALL(primary, GetAntennaActivationTime(ANTENNA_PRIMARY_CHANNEL, _, _)).Times(4).WillRepeatedly(Return(OSResultSuccess));

    EXPECT_CALL(primary, GetTemperature(ANTENNA_BACKUP_CHANNEL, _)).WillOnce(Return(OSResultDeviceNotFound));
    EXPECT_CALL(primary, GetAntennaActivationCount(ANTENNA_BACKUP_CHANNEL, _, _)).Times(4).WillRepeatedly(Return(OSResultDeviceNotFound));
    EXPECT_CALL(primary, GetAntennaActivationTime(ANTENNA_BACKUP_CHANNEL, _, _)).Times(4).WillRepeatedly(Return(OSResultDeviceNotFound));

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
    EXPECT_CALL(primary, GetTemperature(_, _)).WillRepeatedly(Return(OSResultDeviceNotFound));
    EXPECT_CALL(primary, GetAntennaActivationCount(_, _, _)).WillRepeatedly(Return(OSResultDeviceNotFound));
    EXPECT_CALL(primary, GetAntennaActivationTime(_, _, _)).WillRepeatedly(Return(OSResultDeviceNotFound));

    const auto result = driver.GetTelemetry(&driver);
    ASSERT_THAT(result.flags, Eq(0u));
}

TEST_F(AntennaDriverTest, TestAutoDeployAntennaArmsDeploymentSystem)
{
    EXPECT_CALL(primary, ArmDeploymentSystem(ANTENNA_PRIMARY_CHANNEL)).Times(1);
    driver.DeployAntenna(&driver, ANTENNA_PRIMARY_CHANNEL, ANTENNA_AUTO_ID, TimeSpanFromSeconds(10), false);
}

TEST_F(AntennaDriverTest, TestManualDeployAntennaArmsDeploymentSystem)
{
    EXPECT_CALL(primary, ArmDeploymentSystem(ANTENNA_PRIMARY_CHANNEL)).Times(1);
    driver.DeployAntenna(&driver, ANTENNA_PRIMARY_CHANNEL, ANTENNA3_ID, TimeSpanFromSeconds(10), false);
}

TEST_F(AntennaDriverTest, TestDeployAntennaArmingFailure)
{
    EXPECT_CALL(primary, ArmDeploymentSystem(ANTENNA_PRIMARY_CHANNEL)).WillOnce(Return(OSResultDeviceNotFound));
    const auto status = driver.DeployAntenna(&driver, ANTENNA_PRIMARY_CHANNEL, ANTENNA3_ID, TimeSpanFromSeconds(10), false);
    ASSERT_THAT(status, Ne(OSResultSuccess));
}

TEST_F(AntennaDriverTest, TestAutomaticDeploymentSuccess)
{
    ON_CALL(primary, ArmDeploymentSystem(_)).WillByDefault(Return(OSResultSuccess));
    EXPECT_CALL(primary, InitializeAutomaticDeployment(ANTENNA_BACKUP_CHANNEL, _)).WillOnce(Return(OSResultSuccess));
    const auto status = driver.DeployAntenna(&driver, ANTENNA_BACKUP_CHANNEL, ANTENNA_AUTO_ID, TimeSpanFromSeconds(20), false);
    ASSERT_THAT(status, Eq(OSResultSuccess));
}

TEST_F(AntennaDriverTest, TestAutomaticDeploymentFailure)
{
    ON_CALL(primary, ArmDeploymentSystem(_)).WillByDefault(Return(OSResultSuccess));
    EXPECT_CALL(primary, InitializeAutomaticDeployment(ANTENNA_BACKUP_CHANNEL, _)).WillOnce(Return(OSResultDeviceNotFound));
    const auto status = driver.DeployAntenna(&driver, ANTENNA_BACKUP_CHANNEL, ANTENNA_AUTO_ID, TimeSpanFromSeconds(20), false);
    ASSERT_THAT(status, Ne(OSResultSuccess));
}

TEST_F(AntennaDriverTest, TestManualDeploymentSuccess)
{
    ON_CALL(primary, ArmDeploymentSystem(_)).WillByDefault(Return(OSResultSuccess));
    EXPECT_CALL(primary, DeployAntenna(ANTENNA_BACKUP_CHANNEL, ANTENNA3_ID, _, false)).WillOnce(Return(OSResultSuccess));
    const auto status = driver.DeployAntenna(&driver, ANTENNA_BACKUP_CHANNEL, ANTENNA3_ID, TimeSpanFromSeconds(20), false);
    ASSERT_THAT(status, Eq(OSResultSuccess));
}

TEST_F(AntennaDriverTest, TestManualDeploymentFailure)
{
    ON_CALL(primary, ArmDeploymentSystem(_)).WillByDefault(Return(OSResultSuccess));
    EXPECT_CALL(primary, DeployAntenna(ANTENNA_BACKUP_CHANNEL, ANTENNA3_ID, _, false)).WillOnce(Return(OSResultDeviceNotFound));
    const auto status = driver.DeployAntenna(&driver, ANTENNA_BACKUP_CHANNEL, ANTENNA3_ID, TimeSpanFromSeconds(20), false);
    ASSERT_THAT(status, Ne(OSResultSuccess));
}

TEST_F(AntennaDriverTest, TestManualDeploymentSuccessWithOverride)
{
    ON_CALL(primary, ArmDeploymentSystem(_)).WillByDefault(Return(OSResultSuccess));
    EXPECT_CALL(primary, DeployAntenna(ANTENNA_BACKUP_CHANNEL, ANTENNA3_ID, _, true)).WillOnce(Return(OSResultSuccess));
    const auto status = driver.DeployAntenna(&driver, ANTENNA_BACKUP_CHANNEL, ANTENNA3_ID, TimeSpanFromSeconds(20), true);
    ASSERT_THAT(status, Eq(OSResultSuccess));
}

TEST_F(AntennaDriverTest, TestFinishDeploymentCancelsDeployment)
{
    EXPECT_CALL(primary, CancelAntennaDeployment(ANTENNA_PRIMARY_CHANNEL)).Times(1);
    driver.FinishDeployment(&driver, ANTENNA_PRIMARY_CHANNEL);
}

TEST_F(AntennaDriverTest, TestFinishDeploymentDisarmsDeploymentSystem)
{
    ON_CALL(primary, CancelAntennaDeployment(_)).WillByDefault(Return(OSResultSuccess));
    EXPECT_CALL(primary, DisarmDeploymentSystem(ANTENNA_PRIMARY_CHANNEL)).WillOnce(Return(OSResultSuccess));
    const auto result = driver.FinishDeployment(&driver, ANTENNA_PRIMARY_CHANNEL);
    ASSERT_THAT(result, Eq(OSResultSuccess));
}

TEST_F(AntennaDriverTest, TestFinishDeploymentCancelFailure)
{
    ON_CALL(primary, CancelAntennaDeployment(_)).WillByDefault(Return(OSResultDeviceNotFound));
    EXPECT_CALL(primary, DisarmDeploymentSystem(ANTENNA_PRIMARY_CHANNEL)).Times(0);
    const auto result = driver.FinishDeployment(&driver, ANTENNA_PRIMARY_CHANNEL);
    ASSERT_THAT(result, Ne(OSResultSuccess));
}

TEST_F(AntennaDriverTest, TestFinishDeploymentDisarmingFailure)
{
    ON_CALL(primary, CancelAntennaDeployment(_)).WillByDefault(Return(OSResultSuccess));
    ON_CALL(primary, DisarmDeploymentSystem(_)).WillByDefault(Return(OSResultDeviceNotFound));
    const auto result = driver.FinishDeployment(&driver, ANTENNA_PRIMARY_CHANNEL);
    ASSERT_THAT(result, Ne(OSResultSuccess));
}

TEST_F(AntennaDriverTest, TestIsDeploymentActiveNone)
{
    ON_CALL(primary, GetDeploymentStatus(_, _))
        .WillByDefault(Invoke([](AntennaChannel /*channel*/, AntennaDeploymentStatus* status) //
            {
                memset(status, 0, sizeof(*status));
                return OSResultSuccess;
            }));

    const auto status = driver.IsDeploymentActive(&driver, ANTENNA_BACKUP_CHANNEL);
    ASSERT_THAT(status.status, Eq(OSResultSuccess));
    ASSERT_THAT(status.delpoymentInProgress, Eq(false));
}

TEST_F(AntennaDriverTest, TestIsDeploymentActiveSingle)
{
    ON_CALL(primary, GetDeploymentStatus(_, _))
        .WillByDefault(Invoke([](AntennaChannel /*channel*/, AntennaDeploymentStatus* status) //
            {
                memset(status, 0, sizeof(*status));
                status->IsDeploymentActive[3] = true;
                return OSResultSuccess;
            }));

    const auto status = driver.IsDeploymentActive(&driver, ANTENNA_BACKUP_CHANNEL);
    ASSERT_THAT(status.status, Eq(OSResultSuccess));
    ASSERT_THAT(status.delpoymentInProgress, Eq(true));
}

TEST_F(AntennaDriverTest, TestIsDeploymentActiveFailure)
{
    ON_CALL(primary, GetDeploymentStatus(_, _)).WillByDefault(Return(OSResultDeviceNotFound));
    const auto status = driver.IsDeploymentActive(&driver, ANTENNA_BACKUP_CHANNEL);
    ASSERT_THAT(status.status, Ne(OSResultSuccess));
    ASSERT_THAT(status.delpoymentInProgress, Eq(false));
}
