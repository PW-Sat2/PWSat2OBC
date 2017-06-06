#ifndef UNIT_TESTS_BASE_MOCK_EPS_HPP_
#define UNIT_TESTS_BASE_MOCK_EPS_HPP_

#pragma once

#include "gmock/gmock.h"
#include "eps/eps.h"

struct EpsTelemetryProviderMock : devices::eps::IEpsTelemetryProvider
{
    EpsTelemetryProviderMock();
    ~EpsTelemetryProviderMock();
    MOCK_METHOD0(ReadHousekeepingA, Option<devices::eps::hk::ControllerATelemetry>());
    MOCK_METHOD0(ReadHousekeepingB, Option<devices::eps::hk::ControllerBTelemetry>());
};

#endif /* UNIT_TESTS_BASE_MOCK_EPS_HPP_ */
