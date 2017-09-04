#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "OsMock.hpp"
#include "mission/base.hpp"
#include "telemetry/collect_os.hpp"
#include "telemetry/state.hpp"

namespace
{
    using testing::Return;
    using testing::Eq;

    using namespace std::chrono_literals;

    class SystemTelemetryAcquisitionTest : public testing::Test
    {
      protected:
        SystemTelemetryAcquisitionTest();
        mission::UpdateResult Run();
        OSMock os;
        telemetry::TelemetryState state;
        telemetry::SystemTelemetryAcquisition task;
        mission::UpdateDescriptor<telemetry::TelemetryState> descriptor;
    };

    SystemTelemetryAcquisitionTest::SystemTelemetryAcquisitionTest() : task(0), descriptor(task.BuildUpdate())
    {
    }

    mission::UpdateResult SystemTelemetryAcquisitionTest::Run()
    {
        return descriptor.Execute(state);
    }

    TEST_F(SystemTelemetryAcquisitionTest, TestAcquisition)
    {
        auto guard = InstallProxy(&os);
        EXPECT_CALL(os, GetUptime()).WillOnce(Return(0x1234567ms));
        const auto result = Run();
        ASSERT_THAT(result, Eq(mission::UpdateResult::Ok));
        ASSERT_THAT(state.telemetry.IsModified(), Eq(true));
        ASSERT_THAT(state.telemetry.Get<telemetry::OSState>().GetValue().Value(), Eq(0x1234567u / 1000));
    }
}
