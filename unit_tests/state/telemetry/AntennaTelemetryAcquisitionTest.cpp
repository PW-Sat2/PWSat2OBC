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

    struct AntennaTelemetryProviderMock : devices::antenna::IAntennaTelemetryProvider
    {
        MOCK_CONST_METHOD1(GetTelemetry, bool(AntennaTelemetry&));
    };

    class AntennaTelemetryAcquisitionTest : public testing::Test
    {
      protected:
        AntennaTelemetryAcquisitionTest();
        mission::UpdateResult Run();
        telemetry::TelemetryState state;
        telemetry::AntennaTelemetryAcquisition task;
        mission::UpdateDescriptor<telemetry::TelemetryState> descriptor;
    };

    AntennaTelemetryAcquisitionTest::AntennaTelemetryAcquisitionTest() : task(nullptr), descriptor(task.BuildUpdate())
    {
    }

    mission::UpdateResult AntennaTelemetryAcquisitionTest::Run()
    {
        return descriptor.Execute(state);
    }
}
