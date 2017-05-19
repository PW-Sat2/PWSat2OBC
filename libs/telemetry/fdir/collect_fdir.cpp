#include "collect_fdir.hpp"
#include "telemetry/ErrorCounters.hpp"

namespace telemetry
{
    ErrorCounterTelemetryAcquisition::ErrorCounterTelemetryAcquisition(obc::FDIR& fdirProvider) : provider(&fdirProvider)
    {
    }

    mission::UpdateDescriptor<telemetry::TelemetryState> ErrorCounterTelemetryAcquisition::BuildUpdate()
    {
        mission::UpdateDescriptor<telemetry::TelemetryState> descriptor;
        descriptor.name = "ErrorCounting Telemetry Acquisition";
        descriptor.updateProc = UpdateProc;
        descriptor.param = this;
        return descriptor;
    }

    mission::UpdateResult ErrorCounterTelemetryAcquisition::UpdateErrorCountingTelemetry(telemetry::TelemetryState& state)
    {
        const auto& fdir = this->provider->ErrorCounting();
        ErrorCountingTelemetry::Container counters;

        for (size_t i = 0; i < counters.size(); ++i)
        {
            counters[i] = fdir.Current(i);
        }

        state.telemetry.Set(ErrorCountingTelemetry(counters));
        return mission::UpdateResult::Ok;
    }

    mission::UpdateResult ErrorCounterTelemetryAcquisition::UpdateProc(telemetry::TelemetryState& state, void* param)
    {
        auto This = static_cast<ErrorCounterTelemetryAcquisition*>(param);
        return This->UpdateErrorCountingTelemetry(state);
    }
}
