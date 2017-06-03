#ifndef LIBS_TELEMETRY_STATE_HPP
#define LIBS_TELEMETRY_STATE_HPP

#pragma once

#include "ErrorCounters.hpp"
#include "Experiments.hpp"
#include "ProgramState.hpp"
#include "SystemStartup.hpp"
#include "Telemetry.hpp"
#include "comm/CommTelemetry.hpp"
#include "fwd.hpp"
#include "gyro/telemetry.hpp"
#include "state/time/TimeState.hpp"

namespace telemetry
{
    /**
     * @brief This type represents state of telemetry acquisition loop.
     * @ingroup telemetry
     */
    struct TelemetryState
    {
        /**
         * @brief Container for all telemetry elements currently collected by acquisition loop.
         */
        ManagedTelemetry telemetry;
    };

    static_assert(ManagedTelemetry::TotalSerializedSize <= (devices::comm::MaxDownlinkFrameSize - 3), "Telemetry is too large");
}

#endif
