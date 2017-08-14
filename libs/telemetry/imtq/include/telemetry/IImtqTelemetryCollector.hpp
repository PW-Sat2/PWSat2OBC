#ifndef LIBS_TELEMETRY_IIMTQ_TELEMETRY_COLLECTOR_HPP
#define LIBS_TELEMETRY_IIMTQ_TELEMETRY_COLLECTOR_HPP

#pragma once

#include "telemetry/fwd.hpp"

namespace telemetry
{
    struct IImtqDataProvider
    {
        virtual bool GetLastMagnetometerMeasurement(telemetry::ImtqMagnetometerMeasurements& measurements) = 0;
        virtual bool GetLastDipoles(telemetry::ImtqDipoles& dipoles) = 0;
    };

    /**
     * @brief This is an interface for imtq telemetry collector.
     * @ingroup telemetry
     */
    struct IImtqTelemetryCollector
    {
        /**
         * @brief This method updates the passed object with all recently extracted imtq telemetry elements.
         * @param[out] target Reference to object that should be updated with recently extracted telemetry.
         * @return Operation status, true on success, false otherwise.
         */
        virtual bool CaptureTelemetry(ManagedTelemetry& target) = 0;
    };
}

#endif
