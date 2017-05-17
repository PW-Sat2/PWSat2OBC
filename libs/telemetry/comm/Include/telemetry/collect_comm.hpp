#ifndef LIBS_TELEMETRY_COMM_COLLECT_HPP
#define LIBS_TELEMETRY_COMM_COLLECT_HPP

#pragma once

#include "comm/comm.hpp"
#include "mission/base.hpp"
#include "telemetry/state.hpp"

namespace telemetry
{
    /**
     * @brief This task is responsible for acquiring & updating comm hardware telemetry.
     * @telemetry_acquisition
     * @ingroup telemetry
     */
    class CommTelemetryAcquisition : public mission::Update
    {
      public:
        /**
         * @brief ctor.
         * @param[in] comm Reference to comm driver that will provide this module with hardware telemetry
         */
        CommTelemetryAcquisition(devices::comm::ICommTelemetryProvider& comm);

        /**
         * @brief Builds update descriptor for this task.
         * @return Update descriptor - the comm telemetry acquisition update task.
         */
        mission::UpdateDescriptor<telemetry::TelemetryState> BuildUpdate();

        /**
         * @brief Acquires comm telemetry & stores it in passed state object.
         * @param[in] state Object that should be updated with new comm hardware telemetry.
         * @return Telemetry acquisition result.
         */
        mission::UpdateResult UpdateCommTelemetry(telemetry::TelemetryState& state);

      private:
        /**
         * @brief Updates current comm telemetry in global state.
         * @param[in] state Reference to global state.
         * @param[in] param Current execution context.
         */
        static mission::UpdateResult UpdateProc(telemetry::TelemetryState& state, void* param);

        /**
         * @brief Reference to comm driver.
         */
        devices::comm::ICommTelemetryProvider* provider;
    };
}

#endif
