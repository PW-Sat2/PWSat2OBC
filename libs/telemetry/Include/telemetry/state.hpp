#ifndef LIBS_TELEMETRY_STATE_HPP
#define LIBS_TELEMETRY_STATE_HPP

#pragma once

#include "Telemetry.hpp"
#include "comm/CommTelemetry.hpp"
#include "fwd.hpp"
#include "state/time/TimeState.hpp"

namespace telemetry
{
    struct TelemetryState
    {
        ManagedTelemetry telemetry;
    };
}

#endif
