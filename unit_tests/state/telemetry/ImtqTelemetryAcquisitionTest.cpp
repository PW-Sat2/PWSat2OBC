#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "mission/base.hpp"
#include "mock/ImtqTelemetryCollectorMock.hpp"
#include "telemetry/collect_imtq.hpp"
#include "telemetry/state.hpp"

namespace
{
    using testing::Return;
    using testing::_;
    using testing::Eq;
    using testing::Ne;
    using testing::Invoke;

    using namespace devices::imtq;

    class ImtqTelemetryAcquisitionTest : public ::testing::Test
    {
      protected:
        ImtqTelemetryAcquisitionTest();

        mission::UpdateResult Run();

        ImtqTelemetryCollectorMock collector;
        telemetry::ImtqTelemetryAcquisition acquisition;
        telemetry::TelemetryState state;
        mission::UpdateDescriptor<telemetry::TelemetryState> descriptor;
    };

    ImtqTelemetryAcquisitionTest::ImtqTelemetryAcquisitionTest() : acquisition(collector)
    {
        this->descriptor = this->acquisition.BuildUpdate();
    }

    mission::UpdateResult ImtqTelemetryAcquisitionTest::Run()
    {
        return descriptor.Execute(state);
    }

    TEST_F(ImtqTelemetryAcquisitionTest, TestAcquisitionFailure)
    {
        EXPECT_CALL(collector, CaptureTelemetry(_)).WillOnce(Return(false));
        ASSERT_THAT(Run(), Eq(mission::UpdateResult::Warning));
    }

    TEST_F(ImtqTelemetryAcquisitionTest, TestAcquisitionSuccess)
    {
        EXPECT_CALL(collector, CaptureTelemetry(_)).WillOnce(Return(true));
        ASSERT_THAT(Run(), Eq(mission::UpdateResult::Ok));
    }
}
