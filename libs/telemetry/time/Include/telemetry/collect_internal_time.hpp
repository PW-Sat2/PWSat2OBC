#ifndef LIBS_TELEMETRY_COLLECT_INTERNAL_TIME_HPP
#define LIBS_TELEMETRY_COLLECT_INTERNAL_TIME_HPP

#pragma once

#include "mission/base.hpp"
#include "telemetry/state.hpp"
#include "time/ICurrentTime.hpp"

namespace telemetry
{
    /**
     * @brief This task is responsible for acquiring & updating internal time.
     * @telemetry_acquisition
     * @ingroup telemetry
     */
    class InternalTimeTelemetryAcquisition : public mission::Update
    {
      public:
        /**
         * @brief ctor.
         * @param[in] timeProvider Reference to the time provider that keeps track of passage of time.
         */
        InternalTimeTelemetryAcquisition(services::time::ICurrentTime& timeProvider);

        /**
         * @brief Builds update descriptor for this task.
         * @return Update descriptor - the internal time telemetry acquisition task.
         */
        mission::UpdateDescriptor<telemetry::TelemetryState> BuildUpdate();

        /**
         * @brief Acquires internal time & stores it in passed state object.
         * @param[in] state Object that should be updated with new antenna internal time telemetry.
         * @return Telemetry acquisition result.
         */
        mission::UpdateResult UpdateTelemetry(telemetry::TelemetryState& state);

      private:
        /**
         * @brief Updates current internal time in global state.
         * @param[in] state Reference to global state.
         * @param[in] param Current execution context.
         */
        static mission::UpdateResult UpdateProc(telemetry::TelemetryState& state, void* param);

        /**
         * @brief Reference to current time service.
         */
        services::time::ICurrentTime* provider;
    };
}

#endif
