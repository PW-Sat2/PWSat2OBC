#ifndef LIBS_TELEMETRY_FWD_HPP
#define LIBS_TELEMETRY_FWD_HPP

#pragma once

#include "comm/comm.hpp"
#include "state/fwd.hpp"

namespace telemetry
{
    template <typename... Type> class Telemetry;

    typedef Telemetry<state::TimeState, devices::comm::CommTelemetry> ManagedTelemetry;
}

#endif
