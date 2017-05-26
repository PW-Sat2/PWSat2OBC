#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "mission/base.hpp"
#include "mock/comm.hpp"
#include "telemetry/collect_comm.hpp"
#include "telemetry/state.hpp"

namespace
{
    using testing::Return;
    using testing::_;
    using testing::Eq;
    using testing::Ne;
    using testing::Invoke;

    using namespace devices::comm;

    class CommTelemetryAcquisitionTest : public testing::Test
    {
      protected:
        CommTelemetryAcquisitionTest();
        mission::UpdateResult Run();
        CommTelemetryProviderMock comm;
        telemetry::TelemetryState state;
        telemetry::CommTelemetryAcquisition task;
        mission::UpdateDescriptor<telemetry::TelemetryState> descriptor;
    };

    CommTelemetryAcquisitionTest::CommTelemetryAcquisitionTest() : task(comm), descriptor(task.BuildUpdate())
    {
    }

    mission::UpdateResult CommTelemetryAcquisitionTest::Run()
    {
        return descriptor.Execute(state);
    }

    TEST_F(CommTelemetryAcquisitionTest, TestAcquisitionFailure)
    {
        EXPECT_CALL(comm, GetTelemetry(_)).WillOnce(Return(false));
        const auto result = Run();
        ASSERT_THAT(result, Ne(mission::UpdateResult::Ok));
        ASSERT_THAT(state.telemetry.IsModified(), Eq(false));
    }

    TEST_F(CommTelemetryAcquisitionTest, TestAcquisition)
    {
        EXPECT_CALL(comm, GetTelemetry(_)).WillOnce(Return(true));
        const auto result = Run();
        ASSERT_THAT(result, Eq(mission::UpdateResult::Ok));
    }

    TEST_F(CommTelemetryAcquisitionTest, TestAcquisitionStateUpdate)
    {
        EXPECT_CALL(comm, GetTelemetry(_)).WillOnce(Invoke([](auto& telemetry) {
            telemetry = CommTelemetry({}, {}, {IdleState::On, Bitrate::Comm2400bps, true}, {});
            return true;
        }));
        Run();
        ASSERT_THAT(state.telemetry.IsModified(), Eq(true));
    }
}
