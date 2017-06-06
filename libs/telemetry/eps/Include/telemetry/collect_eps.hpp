#ifndef LIBS_TELEMETRY_EPS_COLLECT_HPP
#define LIBS_TELEMETRY_EPS_COLLECT_HPP

#pragma once

#include "eps/eps.h"
#include "mission/base.hpp"
#include "telemetry/state.hpp"

namespace telemetry
{
    /**
     * @brief This task is responsible for acquiring & updating eps hardware telemetry.
     * @telemetry_acquisition
     * @ingroup telemetry
     */
    class EpsTelemetryAcquisition : public mission::Update
    {
      public:
        /**
         * @brief ctor.
         * @param[in] eps Reference to eps driver that will provide this module with hardware telemetry
         */
        EpsTelemetryAcquisition(devices::eps::IEpsTelemetryProvider& eps);

        /**
         * @brief Builds update descriptor for this task.
         * @return Update descriptor - the eps telemetry acquisition update task.
         */
        mission::UpdateDescriptor<telemetry::TelemetryState> BuildUpdate();

        /**
         * @brief Acquires eps telemetry & stores it in passed state object.
         * @param[in] state Object that should be updated with new eps hardware telemetry.
         * @return Telemetry acquisition result.
         */
        mission::UpdateResult UpdateEpsTelemetry(telemetry::TelemetryState& state);

      private:
        /**
         * @brief Updates current eps telemetry in global state.
         * @param[in] state Reference to global state.
         * @param[in] param Current execution context.
         */
        static mission::UpdateResult UpdateProc(telemetry::TelemetryState& state, void* param);

        /**
         * @brief Reference to eps driver.
         */
        devices::eps::IEpsTelemetryProvider* provider;
    };
}

#endif
