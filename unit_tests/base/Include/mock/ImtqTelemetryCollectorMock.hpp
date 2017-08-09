#ifndef MOCK_IMTQ_TELEMETRY_COLLECTOR_MOCK_HPP
#define MOCK_IMTQ_TELEMETRY_COLLECTOR_MOCK_HPP

#pragma once

#include "gmock/gmock.h"
#include "telemetry/IImtqTelemetryCollector.hpp"
#include "telemetry/state.hpp"

struct ImtqTelemetryCollectorMock : public telemetry::IImtqTelemetryCollector
{
    ImtqTelemetryCollectorMock();
    ~ImtqTelemetryCollectorMock();
    MOCK_METHOD1(CaptureTelemetry, bool(telemetry::ManagedTelemetry& target));
};

#endif
