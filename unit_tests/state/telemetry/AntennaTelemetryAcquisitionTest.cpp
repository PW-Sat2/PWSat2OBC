#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "mission/base.hpp"
#include "mock/AntennaMock.hpp"
#include "telemetry/collect_ant.hpp"
#include "telemetry/state.hpp"

namespace
{
    using testing::Return;
    using testing::_;
    using testing::Eq;
    using testing::Ne;
    using testing::Invoke;

    using namespace devices::antenna;

    class AntennaTelemetryAcquisitionTest : public testing::Test
    {
      protected:
        AntennaTelemetryAcquisitionTest();
        mission::UpdateResult Run();
        AntennaMock mock;
        telemetry::TelemetryState state;
        telemetry::AntennaTelemetryAcquisition task;
        mission::UpdateDescriptor<telemetry::TelemetryState> descriptor;
    };

    AntennaTelemetryAcquisitionTest::AntennaTelemetryAcquisitionTest() : task(mock), descriptor(task.BuildUpdate())
    {
    }

    mission::UpdateResult AntennaTelemetryAcquisitionTest::Run()
    {
        return descriptor.Execute(state);
    }

    TEST_F(AntennaTelemetryAcquisitionTest, TestAcquisitionFailure)
    {
        EXPECT_CALL(mock, GetTelemetry(_)).WillOnce(Return(OSResult::IOError));
        const auto result = Run();
        ASSERT_THAT(result, Eq(mission::UpdateResult::Warning));
        ASSERT_THAT(state.telemetry.IsModified(), Eq(false));
    }

    TEST_F(AntennaTelemetryAcquisitionTest, TestAcquisition)
    {
        EXPECT_CALL(mock, GetTelemetry(_)).WillOnce(Return(OSResult::Success));
        const auto result = Run();
        ASSERT_THAT(result, Eq(mission::UpdateResult::Ok));
    }

    TEST_F(AntennaTelemetryAcquisitionTest, TestAcquisitionStateUpdate)
    {
        EXPECT_CALL(mock, GetTelemetry(_)).WillOnce(Invoke([](auto& telemetry) {
            telemetry.SetDeploymentStatus(1);
            return OSResult::Success;
        }));
        Run();
        ASSERT_THAT(state.telemetry.IsModified(), Eq(true));
    }
}
