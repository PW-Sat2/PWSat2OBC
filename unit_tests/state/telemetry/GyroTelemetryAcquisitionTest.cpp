#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "mock/GyroMock.hpp"
#include "telemetry/collect_gyro.hpp"

namespace
{
    using testing::Return;
    using testing::Eq;
    using testing::Ne;
    using namespace devices::gyro;

    class GyroTelemetryAcquisitionTest : public ::testing::Test
    {
      protected:
        GyroTelemetryAcquisitionTest();
        mission::UpdateResult Run();
        GyroscopeMock gyro;
        telemetry::TelemetryState state;
        telemetry::GyroTelemetryAcquisition task;
        mission::UpdateDescriptor<telemetry::TelemetryState> descriptor;
    };

    GyroTelemetryAcquisitionTest::GyroTelemetryAcquisitionTest() : task(gyro), descriptor(task.BuildUpdate())
    {
    }

    mission::UpdateResult GyroTelemetryAcquisitionTest::Run()
    {
        return descriptor.Execute(state);
    }

    TEST_F(GyroTelemetryAcquisitionTest, TestAcquisitionFailure)
    {
        EXPECT_CALL(gyro, read()).WillOnce(Return(Option<GyroscopeTelemetry>::None()));
        const auto result = Run();
        ASSERT_THAT(result, Ne(mission::UpdateResult::Ok));
        ASSERT_THAT(state.telemetry.IsModified(), Eq(false));
    }

    TEST_F(GyroTelemetryAcquisitionTest, TestAcquisition)
    {
        EXPECT_CALL(gyro, read()).WillOnce(Return(Option<GyroscopeTelemetry>::Some(1, 2, 3, 4)));
        const auto result = Run();
        ASSERT_THAT(result, Eq(mission::UpdateResult::Ok));
    }

    TEST_F(GyroTelemetryAcquisitionTest, TestAcquisitionStateUpdate)
    {
        EXPECT_CALL(gyro, read()).WillOnce(Return(Option<GyroscopeTelemetry>::Some(1, 2, 3, 4)));
        Run();
        ASSERT_THAT(state.telemetry.IsModified(), Eq(true));
        const auto telemetry = state.telemetry.Get<GyroscopeTelemetry>();
        ASSERT_THAT(telemetry.X(), Eq(1));
        ASSERT_THAT(telemetry.Y(), Eq(2));
        ASSERT_THAT(telemetry.Z(), Eq(3));
        ASSERT_THAT(telemetry.Temperature(), Eq(4));
    }
}
