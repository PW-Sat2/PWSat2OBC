#include "collect_external_time.hpp"
#include "antenna/telemetry.hpp"
#include "logger/logger.h"

namespace telemetry
{
    ExternalTimeTelemetryAcquisition::ExternalTimeTelemetryAcquisition(devices::rtc::IRTC& rtcDriver) : provider(&rtcDriver)
    {
    }

    mission::UpdateDescriptor<telemetry::TelemetryState> ExternalTimeTelemetryAcquisition::BuildUpdate()
    {
        mission::UpdateDescriptor<telemetry::TelemetryState> descriptor;
        descriptor.name = "Antenna Telemetry Acquisition";
        descriptor.updateProc = UpdateProc;
        descriptor.param = this;
        return descriptor;
    }

    mission::UpdateResult ExternalTimeTelemetryAcquisition::UpdateTelemetry(telemetry::TelemetryState& state)
    {
        devices::rtc::RTCTime telemetry;
        if (OS_RESULT_FAILED(this->provider->ReadTime(telemetry)))
        {
            LOG(LOG_LEVEL_ERROR, "Unable to acquire rtc telemetry. ");
            return mission::UpdateResult::Warning;
        }
        else
        {
            state.telemetry.Set(ExternalTimeTelemetry(telemetry.ToDuration()));
            return mission::UpdateResult::Ok;
        }
    }

    mission::UpdateResult ExternalTimeTelemetryAcquisition::UpdateProc(telemetry::TelemetryState& state, void* param)
    {
        auto This = static_cast<ExternalTimeTelemetryAcquisition*>(param);
        return This->UpdateTelemetry(state);
    }
}
