#include "collect_fs.hpp"
#include "logger/logger.h"

namespace telemetry
{
    FileSystemTelemetryAcquisition::FileSystemTelemetryAcquisition(services::fs::IFileSystem& fs) : provider(&fs)
    {
    }

    mission::UpdateDescriptor<telemetry::TelemetryState> FileSystemTelemetryAcquisition::BuildUpdate()
    {
        mission::UpdateDescriptor<telemetry::TelemetryState> descriptor;
        descriptor.name = "Filesystem Telemetry Acquisition";
        descriptor.updateProc = UpdateProc;
        descriptor.param = this;
        return descriptor;
    }

    mission::UpdateResult FileSystemTelemetryAcquisition::UpdateTelemetry(telemetry::TelemetryState& state)
    {
        const auto size = this->provider->GetFreeSpace("/");
        if (size == std::numeric_limits<std::uint32_t>::max())
        {
            LOG(LOG_LEVEL_ERROR, "Unable to acquire filesystem telemetry. ");
            return mission::UpdateResult::Warning;
        }
        else
        {
            state.telemetry.Set(FileSystemTelemetry(size));
            return mission::UpdateResult::Ok;
        }
    }

    mission::UpdateResult FileSystemTelemetryAcquisition::UpdateProc(telemetry::TelemetryState& state, void* param)
    {
        auto This = static_cast<FileSystemTelemetryAcquisition*>(param);
        return This->UpdateTelemetry(state);
    }
}
