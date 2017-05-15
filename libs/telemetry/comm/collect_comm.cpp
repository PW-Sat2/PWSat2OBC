#include "collect_comm.hpp"
#include "comm/CommTelemetry.hpp"
#include "logger/logger.h"

namespace telemetry
{
    CommTelemetryAcquisition::CommTelemetryAcquisition(devices::comm::ICommTelemetryProvider& comm) : provider(&comm)
    {
    }

    mission::UpdateDescriptor<telemetry::TelemetryState> CommTelemetryAcquisition::BuildUpdate()
    {
        mission::UpdateDescriptor<telemetry::TelemetryState> descriptor;
        descriptor.name = "Comm Telemetry Acquisition";
        descriptor.updateProc = UpdateProc;
        descriptor.param = this;
        return descriptor;
    }

    mission::UpdateResult CommTelemetryAcquisition::UpdateCommTelemetry(telemetry::TelemetryState& state)
    {
        devices::comm::CommTelemetry telemetry;
        if (!this->provider->GetTelemetry(telemetry))
        {
            LOG(LOG_LEVEL_ERROR, "Unable to acquire comm telemetry. ");
            return mission::UpdateResult::Failure;
        }

        state.telemetry.Set(telemetry);
        return mission::UpdateResult::Ok;
    }

    mission::UpdateResult CommTelemetryAcquisition::UpdateProc(telemetry::TelemetryState& state, void* param)
    {
        auto This = static_cast<CommTelemetryAcquisition*>(param);
        return This->UpdateCommTelemetry(state);
    }
}
