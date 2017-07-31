#ifndef LIBS_TELEMETRY_IIMTQ_TELEMETRY_COLLECTOR_HPP
#define LIBS_TELEMETRY_IIMTQ_TELEMETRY_COLLECTOR_HPP

#pragma once

#include "telemetry/fwd.hpp"

namespace telemetry
{
    struct IImtqTelemetryCollector
    {
        virtual bool CaptureTelemetry(ManagedTelemetry& target) = 0;
    };
}

#endif
