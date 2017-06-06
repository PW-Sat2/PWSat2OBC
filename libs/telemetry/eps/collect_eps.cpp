#include "collect_eps.hpp"
#include "logger/logger.h"

namespace telemetry
{
    EpsTelemetryAcquisition::EpsTelemetryAcquisition(devices::eps::IEpsTelemetryProvider& eps) : provider(&eps)
    {
    }

    mission::UpdateDescriptor<telemetry::TelemetryState> EpsTelemetryAcquisition::BuildUpdate()
    {
        mission::UpdateDescriptor<telemetry::TelemetryState> descriptor;
        descriptor.name = "Eps Telemetry Acquisition";
        descriptor.updateProc = UpdateProc;
        descriptor.param = this;
        return descriptor;
    }

    mission::UpdateResult EpsTelemetryAcquisition::UpdateEpsTelemetry(telemetry::TelemetryState& state)
    {
        mission::UpdateResult status = mission::UpdateResult::Ok;
        const auto primary = this->provider->ReadHousekeepingA();
        if (!primary.HasValue)
        {
            LOG(LOG_LEVEL_ERROR, "Unable to acquire primary eps telemetry. ");
            status = mission::UpdateResult::Failure;
        }
        else
        {
            state.telemetry.Set(primary.Value);
        }

        const auto secondary = this->provider->ReadHousekeepingB();
        if (!secondary.HasValue)
        {
            LOG(LOG_LEVEL_ERROR, "Unable to acquire secondary eps telemetry. ");
            status = mission::UpdateResult::Failure;
        }
        else
        {
            state.telemetry.Set(secondary.Value);
        }

        return status;
    }

    mission::UpdateResult EpsTelemetryAcquisition::UpdateProc(telemetry::TelemetryState& state, void* param)
    {
        auto This = static_cast<EpsTelemetryAcquisition*>(param);
        return This->UpdateEpsTelemetry(state);
    }
}
