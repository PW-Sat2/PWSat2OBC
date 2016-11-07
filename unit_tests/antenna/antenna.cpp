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
    AntennaDriverInitialize(&driver, &primary, &i2c);
}

TEST_F(AntennaDriverTest, TestHardResetBothChannels)
{
    EXPECT_CALL(primary, Reset(&i2c, ANTENNA_PRIMARY_CHANNEL)).Times(1);
    EXPECT_CALL(primary, Reset(&i2c, ANTENNA_BACKUP_CHANNEL)).Times(1);
    driver.HardReset(&driver);
}

TEST_F(AntennaDriverTest, TestHardResetStatusSuccess)
{
    EXPECT_CALL(primary, Reset(&i2c, ANTENNA_PRIMARY_CHANNEL)).WillOnce(Return(OSResultSuccess));
    EXPECT_CALL(primary, Reset(&i2c, ANTENNA_BACKUP_CHANNEL)).WillOnce(Return(OSResultSuccess));
    const auto result = driver.HardReset(&driver);
    ASSERT_THAT(result, Eq(OSResultSuccess));
    ASSERT_THAT(driver.primaryChannel.status, Eq(ANTENNA_PORT_OPERATIONAL));
    ASSERT_THAT(driver.secondaryChannel.status, Eq(ANTENNA_PORT_OPERATIONAL));
}

TEST_F(AntennaDriverTest, TestHardResetPrimaryStatusFailure)
{
    EXPECT_CALL(primary, Reset(&i2c, ANTENNA_PRIMARY_CHANNEL)).WillOnce(Return(OSResultDeviceNotFound));
    EXPECT_CALL(primary, Reset(&i2c, ANTENNA_BACKUP_CHANNEL)).WillOnce(Return(OSResultSuccess));
    const auto result = driver.HardReset(&driver);
    ASSERT_THAT(result, Eq(OSResultSuccess));
    ASSERT_THAT(driver.primaryChannel.status, Eq(ANTENNA_PORT_FAILURE));
}

TEST_F(AntennaDriverTest, TestHardResetSecondaryStatusFailure)
{
    EXPECT_CALL(primary, Reset(&i2c, ANTENNA_PRIMARY_CHANNEL)).WillOnce(Return(OSResultSuccess));
    EXPECT_CALL(primary, Reset(&i2c, ANTENNA_BACKUP_CHANNEL)).WillOnce(Return(OSResultDeviceNotFound));
    const auto result = driver.HardReset(&driver);
    ASSERT_THAT(result, Eq(OSResultSuccess));
    ASSERT_THAT(driver.secondaryChannel.status, Eq(ANTENNA_PORT_FAILURE));
}

TEST_F(AntennaDriverTest, TestHardResetBothChannelsFailure)
{
    EXPECT_CALL(primary, Reset(&i2c, ANTENNA_PRIMARY_CHANNEL)).WillOnce(Return(OSResultDeviceNotFound));
    EXPECT_CALL(primary, Reset(&i2c, ANTENNA_BACKUP_CHANNEL)).WillOnce(Return(OSResultDeviceNotFound));
    const auto result = driver.HardReset(&driver);
    ASSERT_THAT(result, Ne(OSResultSuccess));
}

TEST_F(AntennaDriverTest, TestGetTemperature)
{
    EXPECT_CALL(primary, GetTemperature(&i2c, ANTENNA_PRIMARY_CHANNEL, _))
        .WillOnce(Invoke([](I2CBus* communicationBus, AntennaChannel channel, uint16_t* value) //
            {
                UNREFERENCED_PARAMETER(communicationBus);
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
    EXPECT_CALL(primary, GetTemperature(&i2c, ANTENNA_BACKUP_CHANNEL, _)).WillOnce(Return(OSResultIOError));
    uint16_t result;
    const auto status = driver.GetTemperature(&driver, ANTENNA_BACKUP_CHANNEL, &result);
    ASSERT_THAT(status, Ne(OSResultSuccess));
}

TEST_F(AntennaDriverTest, TestGetTelemetryPositiveTestSecondaryChannel)
{
    EXPECT_CALL(primary, GetTemperature(_, ANTENNA_BACKUP_CHANNEL, _)).WillOnce(Return(OSResultSuccess));
    EXPECT_CALL(primary, GetAntennaActivationCount(_, ANTENNA_BACKUP_CHANNEL, _, _)).Times(4).WillRepeatedly(Return(OSResultSuccess));
    EXPECT_CALL(primary, GetAntennaActivationTime(_, ANTENNA_BACKUP_CHANNEL, _, _)).Times(4).WillRepeatedly(Return(OSResultSuccess));

    EXPECT_CALL(primary, GetTemperature(_, ANTENNA_PRIMARY_CHANNEL, _)).WillOnce(Return(OSResultDeviceNotFound));
    EXPECT_CALL(primary, GetAntennaActivationCount(_, ANTENNA_PRIMARY_CHANNEL, _, _))
        .Times(4)
        .WillRepeatedly(Return(OSResultDeviceNotFound));
    EXPECT_CALL(primary, GetAntennaActivationTime(_, ANTENNA_PRIMARY_CHANNEL, _, _))
        .Times(4)
        .WillRepeatedly(Return(OSResultDeviceNotFound));

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
    EXPECT_CALL(primary, GetTemperature(_, ANTENNA_PRIMARY_CHANNEL, _)).WillOnce(Return(OSResultSuccess));
    EXPECT_CALL(primary, GetAntennaActivationCount(_, ANTENNA_PRIMARY_CHANNEL, _, _)).Times(4).WillRepeatedly(Return(OSResultSuccess));
    EXPECT_CALL(primary, GetAntennaActivationTime(_, ANTENNA_PRIMARY_CHANNEL, _, _)).Times(4).WillRepeatedly(Return(OSResultSuccess));

    EXPECT_CALL(primary, GetTemperature(_, ANTENNA_BACKUP_CHANNEL, _)).WillOnce(Return(OSResultDeviceNotFound));
    EXPECT_CALL(primary, GetAntennaActivationCount(_, ANTENNA_BACKUP_CHANNEL, _, _))
        .Times(4)
        .WillRepeatedly(Return(OSResultDeviceNotFound));
    EXPECT_CALL(primary, GetAntennaActivationTime(_, ANTENNA_BACKUP_CHANNEL, _, _)).Times(4).WillRepeatedly(Return(OSResultDeviceNotFound));

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
    EXPECT_CALL(primary, GetTemperature(_, _, _)).WillRepeatedly(Return(OSResultDeviceNotFound));
    EXPECT_CALL(primary, GetAntennaActivationCount(_, _, _, _)).WillRepeatedly(Return(OSResultDeviceNotFound));
    EXPECT_CALL(primary, GetAntennaActivationTime(_, _, _, _)).WillRepeatedly(Return(OSResultDeviceNotFound));

    const auto result = driver.GetTelemetry(&driver);
    ASSERT_THAT(result.flags, Eq(0u));
}

#if 0
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
    I2CBusMock i2c;
};

AntennaChannelStateManagementTest::AntennaChannelStateManagementTest()
{
    AntennaDriverInitialize(&driver, &primary, &i2c);
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
                                ANTENNA_PORT_OPERATIONAL,
                                ANTENNA_PORT_OPERATIONAL),

                            std::make_tuple(OSResultSuccess,
                                OSResultIOError,
                                OSResultSuccess,
                                ANTENNA_PORT_OPERATIONAL,
                                ANTENNA_PORT_OPERATIONAL,
                                ANTENNA_PORT_OPERATIONAL,
                                ANTENNA_PORT_OPERATIONAL),

                            std::make_tuple(OSResultIOError,
                                OSResultSuccess,
                                OSResultSuccess,
                                ANTENNA_PORT_OPERATIONAL,
                                ANTENNA_PORT_OPERATIONAL,
                                ANTENNA_PORT_OPERATIONAL,
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
                                ANTENNA_PORT_OPERATIONAL)), );
#endif
