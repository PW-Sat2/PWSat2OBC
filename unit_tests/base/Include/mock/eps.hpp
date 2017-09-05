#ifndef UNIT_TESTS_BASE_MOCK_EPS_HPP_
#define UNIT_TESTS_BASE_MOCK_EPS_HPP_

#pragma once

#include "gmock/gmock.h"
#include "eps/eps.h"

struct EpsTelemetryProviderMock : devices::eps::IEpsTelemetryProvider
{
    EpsTelemetryProviderMock();
    virtual ~EpsTelemetryProviderMock();
    MOCK_METHOD0(ReadHousekeepingA, Option<devices::eps::hk::ControllerATelemetry>());
    MOCK_METHOD0(ReadHousekeepingB, Option<devices::eps::hk::ControllerBTelemetry>());
};

struct EPSDriverMock : public devices::eps::IEPSDriver
{
    EPSDriverMock();
    virtual ~EPSDriverMock();
    MOCK_METHOD0(ReadHousekeepingA, Option<devices::eps::hk::ControllerATelemetry>());
    MOCK_METHOD0(ReadHousekeepingB, Option<devices::eps::hk::ControllerBTelemetry>());
    MOCK_METHOD1(DisableOverheatSubmode, bool(devices::eps::IEPSDriver::Controller));
};

#endif /* UNIT_TESTS_BASE_MOCK_EPS_HPP_ */
