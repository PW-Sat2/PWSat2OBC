#ifndef LIBS_TELEMETRY_COLLECT_RAM_SCRUBBING_HPP
#define LIBS_TELEMETRY_COLLECT_RAM_SCRUBBING_HPP

#pragma once

#include "mission/base.hpp"
#include "telemetry/state.hpp"

namespace telemetry
{
    /**
     * @brief This task is responsible for acquiring & updating ram scrubbing telemetry.
     * @telemetry_acquisition
     * @ingroup telemetry
     */
    template <typename Scrubber> class RamScrubbingTelemetryAcquisition : public mission::Update
    {
      public:
        /**
         * @brief ctor.
         * @param[in] param unused
         */
        RamScrubbingTelemetryAcquisition(int param);

        /**
         * @brief Builds update descriptor for this task.
         * @return Update descriptor - the ram scrubbing telemetry acquisition update task.
         */
        mission::UpdateDescriptor<telemetry::TelemetryState> BuildUpdate();

      private:
        /**
         * @brief Updates current ram scrubbing telemetry in global state.
         * @param[in] state Reference to global state.
         * @param[in] param Current execution context.
         */
        static mission::UpdateResult UpdateProc(telemetry::TelemetryState& state, void* param);
    };

    template <typename Scrubber> RamScrubbingTelemetryAcquisition<Scrubber>::RamScrubbingTelemetryAcquisition(int /*param*/)
    {
    }

    template <typename Scrubber>
    mission::UpdateDescriptor<telemetry::TelemetryState> RamScrubbingTelemetryAcquisition<Scrubber>::BuildUpdate()
    {
        mission::UpdateDescriptor<telemetry::TelemetryState> descriptor;
        descriptor.name = "RAM Scrubbing Telemetry Acquisition";
        descriptor.updateProc = UpdateProc;
        descriptor.param = this;
        return descriptor;
    }

    template <typename Scrubber>
    mission::UpdateResult RamScrubbingTelemetryAcquisition<Scrubber>::UpdateProc(telemetry::TelemetryState& state, void* /*param*/)
    {
        const auto result = Scrubber::Current();
        RAMScrubbing telemetry(result);
        state.telemetry.Set(telemetry);
        return mission::UpdateResult::Ok;
    }
}

#endif
