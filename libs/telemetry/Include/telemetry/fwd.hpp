#ifndef LIBS_TELEMETRY_FWD_HPP
#define LIBS_TELEMETRY_FWD_HPP

#pragma once

#include "antenna/antenna.h"
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
    class McuTemperature;
    class GpioState;
    class FileSystemTelemetry;
    class InternalTimeTelemetry;
    class ExternalTimeTelemetry;

    struct TelemetryState;

    template <typename... Type> class Telemetry;

    typedef Telemetry<SystemStartup,            //
        ProgramState,                           //
        InternalTimeTelemetry,                  //
        ExternalTimeTelemetry,                  //
        ErrorCountingTelemetry,                 //
        FileSystemTelemetry,                    //
        devices::antenna::AntennaTelemetry,     //
        ExperimentTelemetry,                    //
        devices::gyro::GyroscopeTelemetry,      //
        devices::comm::CommTelemetry,           //
        GpioState,                              //
        McuTemperature,                         //
        devices::eps::hk::ControllerATelemetry, //
        devices::eps::hk::ControllerBTelemetry  //
        >
        ManagedTelemetry;
}

#endif
