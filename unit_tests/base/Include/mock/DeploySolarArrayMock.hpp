#ifndef MOCK_DEPLOY_SOLAR_ARRAY_MOCK_HPP
#define MOCK_DEPLOY_SOLAR_ARRAY_MOCK_HPP

#pragma once

#include "gmock/gmock.h"
#include "mission/sads.hpp"

struct DeploySolarArrayMock : mission::IDeploySolarArray
{
    DeploySolarArrayMock();

    ~DeploySolarArrayMock();

    MOCK_METHOD0(DeploySolarArray, void());
};

#endif // MOCK_DEPLOY_SOLAR_ARRAY_MOCK_HPP
