#ifndef LIBS_MISSION_TELEMETRY_INCLUDE_MISSION_TELEMETRYSERIALIZATION_HPP_
#define LIBS_MISSION_TELEMETRY_INCLUDE_MISSION_TELEMETRYSERIALIZATION_HPP_

#pragma once

#include "mission/base.hpp"
#include "telemetry/state.hpp"

namespace telemetry
{
    /**
     * @brief This task is responsible for observing the telemetry container state and as soon
     * as change is observed prepare its serialized form.
     * @telemetry_acquisition
     * @ingroup telemetry
     */
    class TelemetrySerialization : public mission::Update
    {
      public:
        /**
         * @brief ctor.
         * @param p dummy
         */
        TelemetrySerialization(int p);
        /**
         * @brief Builds update descriptor for this task.
         * @return Action descriptor - the telemetry change save task.
         */
        mission::UpdateDescriptor<TelemetryState> BuildUpdate();

        /**
         * @brief Serializes current telemetry to shared buffer.
         * @param state Reference to global telemetry acquisition state object.
         * @return Operation status.
         */
        mission::UpdateResult SaveTelemetry(TelemetryState& state);

      private:
        static mission::UpdateResult Proxy(TelemetryState& state, void* param);
    };
}

#endif /* LIBS_MISSION_TELEMETRY_INCLUDE_MISSION_TELEMETRYSERIALIZATION_HPP_ */
