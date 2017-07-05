#include "collect_ant.hpp"
#include "antenna/driver.h"
#include "antenna/telemetry.hpp"
#include "logger/logger.h"

namespace telemetry
{
    AntennaTelemetryAcquisition::AntennaTelemetryAcquisition(AntennaDriver& antenna) : provider(&antenna)
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
        if (OS_RESULT_FAILED(this->provider->GetTelemetry(this->provider, telemetry)))
        {
            LOG(LOG_LEVEL_ERROR, "Unable to acquire antenna telemetry. ");
            return mission::UpdateResult::Warning;
        }
        else
        {
            state.telemetry.Set(telemetry);
            return mission::UpdateResult::Ok;
        }
    }

    mission::UpdateResult AntennaTelemetryAcquisition::UpdateProc(telemetry::TelemetryState& state, void* param)
    {
        auto This = static_cast<AntennaTelemetryAcquisition*>(param);
        return This->UpdateTelemetry(state);
    }
}
