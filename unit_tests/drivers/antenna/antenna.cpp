#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "I2C/I2CMock.hpp"
#include "antenna/driver.h"
#include "antenna/telemetry.hpp"
#include "mock/MiniportMock.hpp"
#include "mock/error_counter.hpp"

using testing::Eq;
using testing::Ne;
using testing::_;
using testing::Return;
using testing::Invoke;
using namespace std::chrono_literals;
using devices::antenna::AntennaTelemetry;

namespace
{
    class AntennaDriverTest : public testing::Test
    {
      protected:
        AntennaDriverTest();

        testing::NiceMock<ErrorCountingConfigrationMock> errorsConfig;
        error_counter::ErrorCounting errors;

        AntennaMiniportMock primary;
        I2CBusMock i2c;
        AntennaDriver driver;
    };

    AntennaDriverTest::AntennaDriverTest() : errors{errorsConfig}, driver(errors, &primary, &i2c, &i2c)
    {
    }

    TEST_F(AntennaDriverTest, TestHardResetBothChannels)
    {
        EXPECT_CALL(primary, Reset(_, &i2c, ANTENNA_PRIMARY_CHANNEL)).Times(1);
        EXPECT_CALL(primary, Reset(_, &i2c, ANTENNA_BACKUP_CHANNEL)).Times(1);
        driver.HardReset();
    }

    TEST_F(AntennaDriverTest, TestHardResetStatusSuccess)
    {
        EXPECT_CALL(primary, Reset(_, &i2c, ANTENNA_PRIMARY_CHANNEL)).WillOnce(Return(OSResult::Success));
        EXPECT_CALL(primary, Reset(_, &i2c, ANTENNA_BACKUP_CHANNEL)).WillOnce(Return(OSResult::Success));
        const auto result = driver.HardReset();
        ASSERT_THAT(result, Eq(OSResult::Success));
        ASSERT_THAT(driver.primaryChannel.status, Eq(ANTENNA_PORT_OPERATIONAL));
        ASSERT_THAT(driver.secondaryChannel.status, Eq(ANTENNA_PORT_OPERATIONAL));
    }

    TEST_F(AntennaDriverTest, TestHardResetPrimaryStatusFailure)
    {
        EXPECT_CALL(primary, Reset(_, &i2c, ANTENNA_PRIMARY_CHANNEL)).WillOnce(Return(OSResult::DeviceNotFound));
        EXPECT_CALL(primary, Reset(_, &i2c, ANTENNA_BACKUP_CHANNEL)).WillOnce(Return(OSResult::Success));
        const auto result = driver.HardReset();
        ASSERT_THAT(result, Eq(OSResult::Success));
        ASSERT_THAT(driver.primaryChannel.status, Eq(ANTENNA_PORT_FAILURE));
    }

    TEST_F(AntennaDriverTest, TestHardResetSecondaryStatusFailure)
    {
        EXPECT_CALL(primary, Reset(_, &i2c, ANTENNA_PRIMARY_CHANNEL)).WillOnce(Return(OSResult::Success));
        EXPECT_CALL(primary, Reset(_, &i2c, ANTENNA_BACKUP_CHANNEL)).WillOnce(Return(OSResult::DeviceNotFound));
        const auto result = driver.HardReset();
        ASSERT_THAT(result, Eq(OSResult::Success));
        ASSERT_THAT(driver.secondaryChannel.status, Eq(ANTENNA_PORT_FAILURE));
    }

    TEST_F(AntennaDriverTest, TestResetStatusSuccess)
    {
        driver.primaryChannel.status = ANTENNA_PORT_FAILURE;
        EXPECT_CALL(primary, Reset(_, &i2c, ANTENNA_PRIMARY_CHANNEL)).WillOnce(Return(OSResult::Success));
        const auto result = driver.Reset(ANTENNA_PRIMARY_CHANNEL);
        ASSERT_THAT(result, Eq(OSResult::Success));
        ASSERT_THAT(driver.primaryChannel.status, Eq(ANTENNA_PORT_OPERATIONAL));
    }

    TEST_F(AntennaDriverTest, TestResetStatusFailure)
    {
        EXPECT_CALL(primary, Reset(_, &i2c, ANTENNA_BACKUP_CHANNEL)).WillOnce(Return(OSResult::DeviceNotFound));
        const auto result = driver.Reset(ANTENNA_BACKUP_CHANNEL);
        ASSERT_THAT(result, Ne(OSResult::Success));
        ASSERT_THAT(driver.secondaryChannel.status, Eq(ANTENNA_PORT_FAILURE));
    }

    TEST_F(AntennaDriverTest, TestHardResetBothChannelsFailure)
    {
        EXPECT_CALL(primary, Reset(_, &i2c, ANTENNA_PRIMARY_CHANNEL)).WillOnce(Return(OSResult::DeviceNotFound));
        EXPECT_CALL(primary, Reset(_, &i2c, ANTENNA_BACKUP_CHANNEL)).WillOnce(Return(OSResult::DeviceNotFound));
        const auto result = driver.HardReset();
        ASSERT_THAT(result, Ne(OSResult::Success));
    }

    TEST_F(AntennaDriverTest, TestGetTemperature)
    {
        EXPECT_CALL(primary, GetTemperature(_, &i2c, ANTENNA_PRIMARY_CHANNEL, _))
            .WillOnce(Invoke([](error_counter::AggregatedErrorCounter& error,
                drivers::i2c::II2CBus* communicationBus,
                AntennaChannel channel,
                uint16_t* value) //
                {
                    UNREFERENCED_PARAMETER(error);
                    UNREFERENCED_PARAMETER(communicationBus);
                    UNREFERENCED_PARAMETER(channel);
                    *value = 10;
                    return OSResult::Success;
                }));

        uint16_t result;
        const auto status = driver.GetTemperature(ANTENNA_PRIMARY_CHANNEL, &result);
        ASSERT_THAT(status, Eq(OSResult::Success));
        ASSERT_THAT(result, Eq(10));
    }

    TEST_F(AntennaDriverTest, TestGetTemperatureFailure)
    {
        EXPECT_CALL(primary, GetTemperature(_, &i2c, ANTENNA_BACKUP_CHANNEL, _)).WillOnce(Return(OSResult::IOError));
        uint16_t result;
        const auto status = driver.GetTemperature(ANTENNA_BACKUP_CHANNEL, &result);
        ASSERT_THAT(status, Ne(OSResult::Success));
    }

    TEST_F(AntennaDriverTest, TestGetTelemetryPositiveTest)
    {
        EXPECT_CALL(primary, GetAntennaActivationCount(_, &i2c, _, _, _)).Times(8).WillRepeatedly(Return(OSResult::Success));
        EXPECT_CALL(primary, GetAntennaActivationTime(_, &i2c, _, _, _)).Times(8).WillRepeatedly(Return(OSResult::Success));
        EXPECT_CALL(primary, GetDeploymentStatus(_, &i2c, _, _)).Times(2).WillRepeatedly(Return(OSResult::Success));

        AntennaTelemetry telemetry;
        const auto result = driver.GetTelemetry(telemetry);
        ASSERT_THAT(result, Eq(OSResult::Success));
    }

    TEST_F(AntennaDriverTest, TestGetTelemetryPositiveTestChannels)
    {
        EXPECT_CALL(primary, GetAntennaActivationCount(_, &i2c, ANTENNA_PRIMARY_CHANNEL, ANTENNA1_ID, _))
            .WillOnce(Return(OSResult::Success));
        EXPECT_CALL(primary, GetAntennaActivationCount(_, &i2c, ANTENNA_BACKUP_CHANNEL, ANTENNA1_ID, _))
            .WillOnce(Return(OSResult::Success));
        EXPECT_CALL(primary, GetAntennaActivationCount(_, &i2c, ANTENNA_PRIMARY_CHANNEL, ANTENNA2_ID, _))
            .WillOnce(Return(OSResult::Success));
        EXPECT_CALL(primary, GetAntennaActivationCount(_, &i2c, ANTENNA_BACKUP_CHANNEL, ANTENNA2_ID, _))
            .WillOnce(Return(OSResult::Success));
        EXPECT_CALL(primary, GetAntennaActivationCount(_, &i2c, ANTENNA_PRIMARY_CHANNEL, ANTENNA3_ID, _))
            .WillOnce(Return(OSResult::Success));
        EXPECT_CALL(primary, GetAntennaActivationCount(_, &i2c, ANTENNA_BACKUP_CHANNEL, ANTENNA3_ID, _))
            .WillOnce(Return(OSResult::Success));
        EXPECT_CALL(primary, GetAntennaActivationCount(_, &i2c, ANTENNA_PRIMARY_CHANNEL, ANTENNA4_ID, _))
            .WillOnce(Return(OSResult::Success));
        EXPECT_CALL(primary, GetAntennaActivationCount(_, &i2c, ANTENNA_BACKUP_CHANNEL, ANTENNA4_ID, _))
            .WillOnce(Return(OSResult::Success));

        EXPECT_CALL(primary, GetAntennaActivationTime(_, &i2c, ANTENNA_PRIMARY_CHANNEL, ANTENNA1_ID, _))
            .WillOnce(Return(OSResult::Success));
        EXPECT_CALL(primary, GetAntennaActivationTime(_, &i2c, ANTENNA_BACKUP_CHANNEL, ANTENNA1_ID, _)).WillOnce(Return(OSResult::Success));
        EXPECT_CALL(primary, GetAntennaActivationTime(_, &i2c, ANTENNA_PRIMARY_CHANNEL, ANTENNA2_ID, _))
            .WillOnce(Return(OSResult::Success));
        EXPECT_CALL(primary, GetAntennaActivationTime(_, &i2c, ANTENNA_BACKUP_CHANNEL, ANTENNA2_ID, _)).WillOnce(Return(OSResult::Success));
        EXPECT_CALL(primary, GetAntennaActivationTime(_, &i2c, ANTENNA_PRIMARY_CHANNEL, ANTENNA3_ID, _))
            .WillOnce(Return(OSResult::Success));
        EXPECT_CALL(primary, GetAntennaActivationTime(_, &i2c, ANTENNA_BACKUP_CHANNEL, ANTENNA3_ID, _)).WillOnce(Return(OSResult::Success));
        EXPECT_CALL(primary, GetAntennaActivationTime(_, &i2c, ANTENNA_PRIMARY_CHANNEL, ANTENNA4_ID, _))
            .WillOnce(Return(OSResult::Success));
        EXPECT_CALL(primary, GetAntennaActivationTime(_, &i2c, ANTENNA_BACKUP_CHANNEL, ANTENNA4_ID, _)).WillOnce(Return(OSResult::Success));

        EXPECT_CALL(primary, GetDeploymentStatus(_, &i2c, ANTENNA_PRIMARY_CHANNEL, _)).WillOnce(Return(OSResult::Success));
        EXPECT_CALL(primary, GetDeploymentStatus(_, &i2c, ANTENNA_BACKUP_CHANNEL, _)).WillOnce(Return(OSResult::Success));

        AntennaTelemetry telemetry;
        const auto result = driver.GetTelemetry(telemetry);
        ASSERT_THAT(result, Eq(OSResult::Success));
    }

    TEST_F(AntennaDriverTest, TestGetTelemetryFailureOnDeploymentStatusPrimaryChannel)
    {
        EXPECT_CALL(primary, GetAntennaActivationCount(_, &i2c, _, _, _)).Times(8).WillRepeatedly(Return(OSResult::Success));
        EXPECT_CALL(primary, GetAntennaActivationTime(_, &i2c, _, _, _)).Times(8).WillRepeatedly(Return(OSResult::Success));
        EXPECT_CALL(primary, GetDeploymentStatus(_, &i2c, _, _)).WillOnce(Return(OSResult::IOError)).WillOnce(Return(OSResult::Success));

        AntennaTelemetry telemetry;
        const auto result = driver.GetTelemetry(telemetry);
        ASSERT_THAT(result, Ne(OSResult::Success));
    }

    TEST_F(AntennaDriverTest, TestGetTelemetryFailureOnDeploymentStatusBackupChannel)
    {
        EXPECT_CALL(primary, GetAntennaActivationCount(_, &i2c, _, _, _)).Times(8).WillRepeatedly(Return(OSResult::Success));
        EXPECT_CALL(primary, GetAntennaActivationTime(_, &i2c, _, _, _)).Times(8).WillRepeatedly(Return(OSResult::Success));
        EXPECT_CALL(primary, GetDeploymentStatus(_, &i2c, _, _)).WillOnce(Return(OSResult::Success)).WillOnce(Return(OSResult::IOError));

        AntennaTelemetry telemetry;
        const auto result = driver.GetTelemetry(telemetry);
        ASSERT_THAT(result, Ne(OSResult::Success));
    }

    TEST_F(AntennaDriverTest, TestGetTelemetryFailureActivationCountPrimaryChannel)
    {
        EXPECT_CALL(primary, GetAntennaActivationCount(_, &i2c, ANTENNA_PRIMARY_CHANNEL, _, _)).WillOnce(Return(OSResult::IOError));
        EXPECT_CALL(primary, GetAntennaActivationCount(_, &i2c, ANTENNA_PRIMARY_CHANNEL, _, _))
            .Times(3)
            .WillRepeatedly(Return(OSResult::Success))
            .RetiresOnSaturation();

        EXPECT_CALL(primary, GetAntennaActivationCount(_, &i2c, ANTENNA_BACKUP_CHANNEL, _, _)).WillRepeatedly(Return(OSResult::Success));

        EXPECT_CALL(primary, GetAntennaActivationTime(_, &i2c, _, _, _)).Times(8).WillRepeatedly(Return(OSResult::Success));
        EXPECT_CALL(primary, GetDeploymentStatus(_, &i2c, _, _)).Times(2).WillRepeatedly(Return(OSResult::Success));

        AntennaTelemetry telemetry;
        const auto result = driver.GetTelemetry(telemetry);
        ASSERT_THAT(result, Ne(OSResult::Success));
    }

    TEST_F(AntennaDriverTest, TestGetTelemetryFailureActivationCountBackupChannel)
    {
        EXPECT_CALL(primary, GetAntennaActivationCount(_, &i2c, ANTENNA_BACKUP_CHANNEL, _, _)).WillOnce(Return(OSResult::IOError));
        EXPECT_CALL(primary, GetAntennaActivationCount(_, &i2c, ANTENNA_BACKUP_CHANNEL, _, _))
            .Times(3)
            .WillRepeatedly(Return(OSResult::Success))
            .RetiresOnSaturation();

        EXPECT_CALL(primary, GetAntennaActivationCount(_, &i2c, ANTENNA_PRIMARY_CHANNEL, _, _)).WillRepeatedly(Return(OSResult::Success));
        EXPECT_CALL(primary, GetAntennaActivationTime(_, &i2c, _, _, _)).Times(8).WillRepeatedly(Return(OSResult::Success));
        EXPECT_CALL(primary, GetDeploymentStatus(_, &i2c, _, _)).Times(2).WillRepeatedly(Return(OSResult::Success));

        AntennaTelemetry telemetry;
        const auto result = driver.GetTelemetry(telemetry);
        ASSERT_THAT(result, Ne(OSResult::Success));
    }

    TEST_F(AntennaDriverTest, TestGetTelemetryFailureActivationTimes)
    {
        EXPECT_CALL(primary, GetAntennaActivationCount(_, &i2c, _, _, _)).Times(8).WillRepeatedly(Return(OSResult::Success));
        EXPECT_CALL(primary, GetAntennaActivationTime(_, &i2c, _, _, _)).WillOnce(Return(OSResult::IOError));
        EXPECT_CALL(primary, GetAntennaActivationTime(_, &i2c, _, _, _))
            .Times(7)
            .WillRepeatedly(Return(OSResult::Success))
            .RetiresOnSaturation();
        EXPECT_CALL(primary, GetDeploymentStatus(_, &i2c, _, _)).Times(2).WillRepeatedly(Return(OSResult::Success));

        AntennaTelemetry telemetry;
        const auto result = driver.GetTelemetry(telemetry);
        ASSERT_THAT(result, Ne(OSResult::Success));
    }
}
