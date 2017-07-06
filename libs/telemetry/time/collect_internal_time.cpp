#include "collect_internal_time.hpp"
#include "antenna/telemetry.hpp"
#include "logger/logger.h"

namespace telemetry
{
    InternalTimeTelemetryAcquisition::InternalTimeTelemetryAcquisition(services::time::ICurrentTime& timeProvider) : provider(&timeProvider)
    {
    }

    mission::UpdateDescriptor<telemetry::TelemetryState> InternalTimeTelemetryAcquisition::BuildUpdate()
    {
        mission::UpdateDescriptor<telemetry::TelemetryState> descriptor;
        descriptor.name = "Internal time telemetry acquisition";
        descriptor.updateProc = UpdateProc;
        descriptor.param = this;
        return descriptor;
    }

    mission::UpdateResult InternalTimeTelemetryAcquisition::UpdateTelemetry(telemetry::TelemetryState& state)
    {
        const auto result = this->provider->GetCurrentTime();
        if (!result.HasValue)
        {
            LOG(LOG_LEVEL_ERROR, "Unable to internal time telemetry. ");
            return mission::UpdateResult::Warning;
        }
        else
        {
            state.telemetry.Set(InternalTimeTelemetry(result.Value));
            return mission::UpdateResult::Ok;
        }
    }

    mission::UpdateResult InternalTimeTelemetryAcquisition::UpdateProc(telemetry::TelemetryState& state, void* param)
    {
        auto This = static_cast<InternalTimeTelemetryAcquisition*>(param);
        return This->UpdateTelemetry(state);
    }
}
