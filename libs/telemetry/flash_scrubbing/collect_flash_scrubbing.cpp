#include "collect_flash_scrubbing.hpp"
#include "logger/logger.h"

namespace telemetry
{
    FlashScrubbingTelemetryAcquisition::FlashScrubbingTelemetryAcquisition(obc::OBCScrubbing& scrubber) : provider(&scrubber)
    {
    }

    mission::UpdateDescriptor<telemetry::TelemetryState> FlashScrubbingTelemetryAcquisition::BuildUpdate()
    {
        mission::UpdateDescriptor<telemetry::TelemetryState> descriptor;
        descriptor.name = "Flash Scrubbing Telemetry Acquisition";
        descriptor.updateProc = UpdateProc;
        descriptor.param = this;
        return descriptor;
    }

    mission::UpdateResult FlashScrubbingTelemetryAcquisition::UpdateTelemetry(telemetry::TelemetryState& state)
    {
        const auto result = this->provider->Status();
        FlashPrimarySlotsScrubbing primary(result.PrimarySlots.Offset);
        FlashSecondarySlotsScrubbing secondary(result.SecondarySlots.Offset);
        state.telemetry.Set(primary);
        state.telemetry.Set(secondary);
        return mission::UpdateResult::Ok;
    }

    mission::UpdateResult FlashScrubbingTelemetryAcquisition::UpdateProc(telemetry::TelemetryState& state, void* param)
    {
        auto This = static_cast<FlashScrubbingTelemetryAcquisition*>(param);
        return This->UpdateTelemetry(state);
    }
}
