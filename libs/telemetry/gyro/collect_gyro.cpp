#include "collect_gyro.hpp"
#include "logger/logger.h"

namespace telemetry
{
    GyroTelemetryAcquisition::GyroTelemetryAcquisition(devices::gyro::IGyroscopeDriver& gyro) : provider(&gyro)
    {
    }

    mission::UpdateDescriptor<telemetry::TelemetryState> GyroTelemetryAcquisition::BuildUpdate()
    {
        mission::UpdateDescriptor<telemetry::TelemetryState> descriptor;
        descriptor.name = "Gyroscope Telemetry Acquisition";
        descriptor.updateProc = UpdateProc;
        descriptor.param = this;
        return descriptor;
    }

    mission::UpdateResult GyroTelemetryAcquisition::UpdateCommTelemetry(telemetry::TelemetryState& state)
    {
        auto status = this->provider->read();
        if (!status.HasValue)
        {
            LOG(LOG_LEVEL_ERROR, "Unable to acquire gyroscope telemetry. ");
            return mission::UpdateResult::Warning;
        }

        state.telemetry.Set(status.Value);
        return mission::UpdateResult::Ok;
    }

    mission::UpdateResult GyroTelemetryAcquisition::UpdateProc(telemetry::TelemetryState& state, void* param)
    {
        auto This = static_cast<GyroTelemetryAcquisition*>(param);
        return This->UpdateCommTelemetry(state);
    }
}
