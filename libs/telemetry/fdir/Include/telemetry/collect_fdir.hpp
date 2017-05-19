#ifndef LIBS_TELEMETRY_FDIR_COLLECT_HPP
#define LIBS_TELEMETRY_FDIR_COLLECT_HPP

#pragma once

#include "mission/base.hpp"
#include "obc/fdir.hpp"
#include "telemetry/state.hpp"

namespace telemetry
{
    /**
     * @brief This task is responsible for acquiring & updating error counters telemetry.
     * @telemetry_acquisition
     * @ingroup telemetry
     */
    class ErrorCounterTelemetryAcquisition : public mission::Update
    {
      public:
        /**
         * @brief ctor.
         * @param[in] fdirProvider Reference to error counter container that will provide this module
         * with error counters' values.
         */
        ErrorCounterTelemetryAcquisition(obc::FDIR& fdirProvider);

        /**
         * @brief Builds update descriptor for this task.
         * @return Update descriptor - the error counter telemetry acquisition update task.
         */
        mission::UpdateDescriptor<telemetry::TelemetryState> BuildUpdate();

        /**
         * @brief Acquires error counter telemetry & stores it in passed state object.
         * @param[in] state Object that should be updated with new gyroscope hardware telemetry.
         * @return Telemetry acquisition result.
         */
        mission::UpdateResult UpdateErrorCountingTelemetry(telemetry::TelemetryState& state);

      private:
        /**
         * @brief Updates current error counter telemetry in global state.
         * @param[in] state Reference to global state.
         * @param[in] param Current execution context.
         */
        static mission::UpdateResult UpdateProc(telemetry::TelemetryState& state, void* param);

        /**
         * @brief Reference to error counter container.
         */
        obc::FDIR* provider;
    };
}

#endif
