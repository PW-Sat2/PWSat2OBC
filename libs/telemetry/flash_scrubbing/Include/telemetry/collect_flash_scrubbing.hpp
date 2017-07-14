#ifndef LIBS_TELEMETRY_COLLECT_FLASH_SCRUBBING_HPP
#define LIBS_TELEMETRY_COLLECT_FLASH_SCRUBBING_HPP

#pragma once

#include "mission/base.hpp"
#include "obc/scrubbing.hpp"
#include "telemetry/state.hpp"

namespace telemetry
{
    /**
     * @brief This task is responsible for acquiring & updating flash scrubbing telemetry.
     * @telemetry_acquisition
     * @ingroup telemetry
     */
    class FlashScrubbingTelemetryAcquisition : public mission::Update
    {
      public:
        /**
         * @brief ctor.
         * @param[in] scrubber Reference to flash scrubber that will provide this module with telemetry
         */
        FlashScrubbingTelemetryAcquisition(obc::OBCScrubbing& scrubber);

        /**
         * @brief Builds update descriptor for this task.
         * @return Update descriptor - the flash scrubbing telemetry acquisition update task.
         */
        mission::UpdateDescriptor<telemetry::TelemetryState> BuildUpdate();

        /**
         * @brief Acquires flash scrubbing telemetry & stores it in passed state object.
         * @param[in] state Object that should be updated with new flash scrubbing telemetry.
         * @return Telemetry acquisition result.
         */
        mission::UpdateResult UpdateTelemetry(telemetry::TelemetryState& state);

      private:
        /**
         * @brief Updates current flash scrubbing telemetry in global state.
         * @param[in] state Reference to global state.
         * @param[in] param Current execution context.
         */
        static mission::UpdateResult UpdateProc(telemetry::TelemetryState& state, void* param);

        /**
         * @brief Reference to flash scrubber.
         */
        obc::OBCScrubbing* provider;
    };
}

#endif
