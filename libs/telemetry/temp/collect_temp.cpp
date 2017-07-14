#include "collect_temp.hpp"
#include "telemetry/BasicTelemetry.hpp"

namespace telemetry
{
    McuTempTelemetryAcquisition::McuTempTelemetryAcquisition(temp::ITemperatureReader& reader) : provider(&reader)
    {
    }

    mission::UpdateDescriptor<telemetry::TelemetryState> McuTempTelemetryAcquisition::BuildUpdate()
    {
        mission::UpdateDescriptor<telemetry::TelemetryState> descriptor;
        descriptor.name = "Mcu temperature acquisition";
        descriptor.updateProc = UpdateProc;
        descriptor.param = this;
        return descriptor;
    }

    mission::UpdateResult McuTempTelemetryAcquisition::UpdateMcuTempTelemetry(telemetry::TelemetryState& state)
    {
        const auto value = this->provider->ReadRaw();
        state.telemetry.Set(McuTemperature(value));
        return mission::UpdateResult::Ok;
    }

    mission::UpdateResult McuTempTelemetryAcquisition::UpdateProc(telemetry::TelemetryState& state, void* param)
    {
        auto This = static_cast<McuTempTelemetryAcquisition*>(param);
        return This->UpdateMcuTempTelemetry(state);
    }
}
