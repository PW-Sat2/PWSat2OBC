#ifndef LIBS_TELEMETRY_STATE_HPP
#define LIBS_TELEMETRY_STATE_HPP

#pragma once

#include "Telemetry.hpp"
#include "comm/CommTelemetry.hpp"
#include "fwd.hpp"
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
}

#endif
