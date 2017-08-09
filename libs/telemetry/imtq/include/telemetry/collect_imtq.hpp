#ifndef LIBS_TELEMETRY_IMTQ_COLLECT_IMTQ_HPP
#define LIBS_TELEMETRY_IMTQ_COLLECT_IMTQ_HPP

#pragma once

#include "mission/base.hpp"
#include "telemetry/state.hpp"

namespace telemetry
{
    struct IImtqTelemetryCollector;

    /**
     * @brief This task is responsible for acquiring & updating imtq hardware telemetry.
     * @telemetry_acquisition
     * @ingroup telemetry
     */
    class ImtqTelemetryAcquisition : public mission::Update
    {
      public:
        /**
         * @brief ctor.
         * @param[in] collector Reference to imtq driver proxy that will provide this module with hardware telemetry
         */
        ImtqTelemetryAcquisition(IImtqTelemetryCollector& collector);

        /**
         * @brief Builds update descriptor for this task.
         * @return Update descriptor - the imtq telemetry acquisition update task.
         */
        mission::UpdateDescriptor<telemetry::TelemetryState> BuildUpdate();

        /**
         * @brief Acquires imtq telemetry & stores it in passed state object.
         * @param[in] state Object that should be updated with new imtq hardware telemetry.
         * @return Telemetry acquisition result.
         */
        mission::UpdateResult UpdateTelemetry(telemetry::TelemetryState& state);

      private:
        /**
         * @brief Updates current imtq telemetry in global state.
         * @param[in] state Reference to global state.
         * @param[in] param Current execution context.
         */
        static mission::UpdateResult UpdateProc(telemetry::TelemetryState& state, void* param);

        /**
         * @brief Reference to imtq telemetry collector.
         */
        IImtqTelemetryCollector* provider;
    };
}

#endif
