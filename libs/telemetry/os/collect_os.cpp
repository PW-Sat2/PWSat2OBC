#include "telemetry/collect_os.hpp"
#include "base/os.h"

namespace telemetry
{
    SystemTelemetryAcquisition::SystemTelemetryAcquisition(int /*value*/)
    {
    }

    mission::UpdateDescriptor<telemetry::TelemetryState> SystemTelemetryAcquisition::BuildUpdate()
    {
        mission::UpdateDescriptor<telemetry::TelemetryState> descriptor;
        descriptor.name = "OS Telemetry Acquisition";
        descriptor.updateProc = UpdateProc;
        descriptor.param = nullptr;
        return descriptor;
    }

    mission::UpdateResult SystemTelemetryAcquisition::UpdateProc(telemetry::TelemetryState& state, void* /*param*/)
    {
        const auto uptime = std::chrono::duration_cast<std::chrono::seconds>(System::GetUptime());
        state.telemetry.Set(OSState(uptime.count()));
        return mission::UpdateResult::Ok;
    }
}
