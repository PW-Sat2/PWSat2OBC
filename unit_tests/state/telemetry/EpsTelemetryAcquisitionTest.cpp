#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "mission/base.hpp"
#include "mock/eps.hpp"
#include "telemetry/collect_eps.hpp"
#include "telemetry/state.hpp"

namespace
{
    using testing::Return;
    using testing::_;
    using testing::Eq;
    using testing::Ne;

    using namespace devices::eps::hk;

    class EpsTelemetryAcquisitionTest : public testing::Test
    {
      protected:
        EpsTelemetryAcquisitionTest();
        mission::UpdateResult Run();
        EpsTelemetryProviderMock eps;
        telemetry::TelemetryState state;
        telemetry::EpsTelemetryAcquisition task;
        mission::UpdateDescriptor<telemetry::TelemetryState> descriptor;
    };

    EpsTelemetryAcquisitionTest::EpsTelemetryAcquisitionTest() : task(eps), descriptor(task.BuildUpdate())
    {
    }

    mission::UpdateResult EpsTelemetryAcquisitionTest::Run()
    {
        return descriptor.Execute(state);
    }

    TEST_F(EpsTelemetryAcquisitionTest, TestPrimaryAcquisitionFailure)
    {
        EXPECT_CALL(eps, ReadHousekeepingA()).WillOnce(Return(Option<ControllerATelemetry>::None()));
        EXPECT_CALL(eps, ReadHousekeepingB()).WillOnce(Return(Option<ControllerBTelemetry>::None()));
        const auto result = Run();
        ASSERT_THAT(result, Ne(mission::UpdateResult::Ok));
        ASSERT_THAT(state.telemetry.IsModified(), Eq(false));
    }

    TEST_F(EpsTelemetryAcquisitionTest, TestPartialAcquisitionA)
    {
        EXPECT_CALL(eps, ReadHousekeepingA()).WillOnce(Return(Option<ControllerATelemetry>::Some(ControllerATelemetry{})));
        EXPECT_CALL(eps, ReadHousekeepingB()).WillOnce(Return(Option<ControllerBTelemetry>::None()));
        const auto result = Run();
        ASSERT_THAT(result, Ne(mission::UpdateResult::Ok));
        ASSERT_THAT(state.telemetry.IsModified(), Eq(true));
    }

    TEST_F(EpsTelemetryAcquisitionTest, TestPartialAcquisitionB)
    {
        EXPECT_CALL(eps, ReadHousekeepingA()).WillOnce(Return(Option<ControllerATelemetry>::None()));
        EXPECT_CALL(eps, ReadHousekeepingB()).WillOnce(Return(Option<ControllerBTelemetry>::Some(ControllerBTelemetry{})));
        const auto result = Run();
        ASSERT_THAT(result, Ne(mission::UpdateResult::Ok));
        ASSERT_THAT(state.telemetry.IsModified(), Eq(true));
    }
}
