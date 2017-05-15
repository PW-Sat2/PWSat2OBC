#ifndef LIBS_TELEMETRY_COMM_COLLECT_HPP
#define LIBS_TELEMETRY_COMM_COLLECT_HPP

#pragma once

#include "comm/comm.hpp"
#include "mission/base.hpp"
#include "telemetry/state.hpp"

namespace telemetry
{
    class CommTelemetryAcquisition : public mission::Update
    {
      public:
        CommTelemetryAcquisition(devices::comm::ICommTelemetryProvider& comm);

        mission::UpdateDescriptor<telemetry::TelemetryState> BuildUpdate();

        mission::UpdateResult UpdateCommTelemetry(telemetry::TelemetryState& state);

      private:
        /**
         * @brief Updates current time in global mission state.
         * @param[in] state Reference to global mission state.
         * @param[in] param Current execution context.
         */
        static mission::UpdateResult UpdateProc(telemetry::TelemetryState& state, void* param);

        devices::comm::ICommTelemetryProvider* provider;
    };
}

#endif
