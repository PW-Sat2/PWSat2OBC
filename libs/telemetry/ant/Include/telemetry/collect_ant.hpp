#ifndef LIBS_TELEMETRY_ANT_COLLECT_HPP
#define LIBS_TELEMETRY_ANT_COLLECT_HPP

#pragma once

#include "antenna/antenna.h"
#include "mission/base.hpp"
#include "telemetry/state.hpp"

namespace telemetry
{
    /**
     * @brief This task is responsible for acquiring & updating antenna hardware telemetry.
     * @telemetry_acquisition
     * @ingroup telemetry
     */
    class AntennaTelemetryAcquisition : public mission::Update
    {
      public:
        /**
         * @brief ctor.
         * @param[in] antenna Reference to antenna driver that will provide this module with hardware telemetry
         */
        AntennaTelemetryAcquisition(AntennaDriver& antenna);

        /**
         * @brief Builds update descriptor for this task.
         * @return Update descriptor - the antenna telemetry acquisition update task.
         */
        mission::UpdateDescriptor<telemetry::TelemetryState> BuildUpdate();

        /**
         * @brief Acquires antenna telemetry & stores it in passed state object.
         * @param[in] state Object that should be updated with new antenna hardware telemetry.
         * @return Telemetry acquisition result.
         */
        mission::UpdateResult UpdateTelemetry(telemetry::TelemetryState& state);

      private:
        /**
         * @brief Updates current antenna telemetry in global state.
         * @param[in] state Reference to global state.
         * @param[in] param Current execution context.
         */
        static mission::UpdateResult UpdateProc(telemetry::TelemetryState& state, void* param);

        /**
         * @brief Reference to antenna driver.
         */
        AntennaDriver* provider;
    };
}

#endif
