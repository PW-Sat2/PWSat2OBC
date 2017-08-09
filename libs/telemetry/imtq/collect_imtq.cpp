#include "IImtqTelemetryCollector.hpp"
#include "collect_imtq.hpp"
#include "logger/logger.h"

namespace telemetry
{
    ImtqTelemetryAcquisition::ImtqTelemetryAcquisition(IImtqTelemetryCollector& collector) : provider(&collector)
    {
    }

    mission::UpdateDescriptor<telemetry::TelemetryState> ImtqTelemetryAcquisition::BuildUpdate()
    {
        mission::UpdateDescriptor<telemetry::TelemetryState> descriptor;
        descriptor.name = "Imtq Telemetry Acquisition";
        descriptor.updateProc = UpdateProc;
        descriptor.param = this;
        return descriptor;
    }

    mission::UpdateResult ImtqTelemetryAcquisition::UpdateTelemetry(telemetry::TelemetryState& state)
    {
        if (this->provider->CaptureTelemetry(state.telemetry))
        {
            return mission::UpdateResult::Ok;
        }
        else
        {
            LOG(LOG_LEVEL_ERROR, "Unable to acquire imtq telemetry. ");
            return mission::UpdateResult::Warning;
        }
    }

    mission::UpdateResult ImtqTelemetryAcquisition::UpdateProc(telemetry::TelemetryState& state, void* param)
    {
        auto This = static_cast<ImtqTelemetryAcquisition*>(param);
        return This->UpdateTelemetry(state);
    }
}
