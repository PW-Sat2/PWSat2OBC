#include "collect_ant.hpp"
#include "antenna/driver.h"
#include "antenna/telemetry.hpp"
#include "logger/logger.h"

namespace telemetry
{
    AntennaTelemetryAcquisition::AntennaTelemetryAcquisition(devices::antenna::IAntennaTelemetryProvider*)
    {
    }

    mission::UpdateDescriptor<telemetry::TelemetryState> AntennaTelemetryAcquisition::BuildUpdate()
    {
        mission::UpdateDescriptor<telemetry::TelemetryState> descriptor;
        descriptor.name = "Antenna Telemetry Acquisition";
        descriptor.updateProc = UpdateProc;
        descriptor.param = this;
        return descriptor;
    }

    mission::UpdateResult AntennaTelemetryAcquisition::UpdateTelemetry(telemetry::TelemetryState& state)
    {
        devices::antenna::AntennaTelemetry telemetry;
        state.telemetry.Set(telemetry);
        return mission::UpdateResult::Ok;
    }

    mission::UpdateResult AntennaTelemetryAcquisition::UpdateProc(telemetry::TelemetryState& state, void* param)
    {
        auto This = static_cast<AntennaTelemetryAcquisition*>(param);
        return This->UpdateTelemetry(state);
    }
}
