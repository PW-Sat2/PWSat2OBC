#ifndef LIBS_TELEMETRY_FWD_HPP
#define LIBS_TELEMETRY_FWD_HPP

#pragma once

#include "comm/comm.hpp"
#include "eps/eps.h"
#include "gyro/gyro.h"
#include "state/fwd.hpp"

namespace telemetry
{
    class SystemStartup;
    class ProgramState;
    class ErrorCountingTelemetry;
    class ExperimentTelemetry;

    struct TelemetryState;

    template <typename... Type> class Telemetry;

    typedef Telemetry<SystemStartup,            //
        ProgramState,                           //
        state::TimeState,                       //
        ErrorCountingTelemetry,                 //
        ExperimentTelemetry,                    //
        devices::comm::CommTelemetry,           //
        devices::gyro::GyroscopeTelemetry,      //
        devices::eps::hk::ControllerATelemetry, //
        devices::eps::hk::ControllerBTelemetry  //
        >
        ManagedTelemetry;
}

#endif
