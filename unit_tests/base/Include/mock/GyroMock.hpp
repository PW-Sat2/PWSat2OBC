#ifndef MOCK_GYRO_MOCK_HPP
#define MOCK_GYRO_MOCK_HPP

#pragma once

#include "gmock/gmock.h"
#include "gyro/gyro.h"
#include "gyro/telemetry.hpp"
#include "utils.h"

struct GyroscopeMock : public devices::gyro::IGyroscopeDriver
{
    GyroscopeMock();
    ~GyroscopeMock();
    MOCK_METHOD0(read, Option<devices::gyro::GyroscopeTelemetry>());
};

#endif
