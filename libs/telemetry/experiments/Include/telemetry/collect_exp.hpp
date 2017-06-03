#ifndef LIBS_TELEMETRY_EXP_COLLECT_HPP
#define LIBS_TELEMETRY_EXP_COLLECT_HPP

#pragma once

#include "experiments/experiments.h"
#include "mission/base.hpp"
#include "telemetry/state.hpp"

namespace telemetry
{
    /**
     * @brief This task is responsible for acquiring & updating experiment hardware telemetry.
     * @telemetry_acquisition
     * @ingroup telemetry
     */
    class ExperimentTelemetryAcquisition : public mission::Update
    {
      public:
        /**
         * @brief ctor.
         * @param[in] experiment Reference to experiment controller that will provide this module with experiment state.
         */
        ExperimentTelemetryAcquisition(experiments::IExperimentController& experiment);

        /**
         * @brief Builds update descriptor for this task.
         * @return Update descriptor - the experiment telemetry acquisition update task.
         */
        mission::UpdateDescriptor<telemetry::TelemetryState> BuildUpdate();

        /**
         * @brief Acquires experiment telemetry & stores it in passed state object.
         * @param[in] state Object that should be updated with new experiment hardware telemetry.
         * @return Telemetry acquisition result.
         */
        mission::UpdateResult UpdateExperimentTelemetry(telemetry::TelemetryState& state);

      private:
        /**
         * @brief Updates current experiment telemetry in global state.
         * @param[in] state Reference to global state.
         * @param[in] param Current execution context.
         */
        static mission::UpdateResult UpdateProc(telemetry::TelemetryState& state, void* param);

        /**
         * @brief Reference to experiment driver.
         */
        experiments::IExperimentController* provider;
    };
}

#endif
