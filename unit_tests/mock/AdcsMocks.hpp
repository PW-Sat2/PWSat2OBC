#ifndef ADCS_COORDINATOR_MOCK_HPP
#define ADCS_COORDINATOR_MOCK_HPP

#pragma once

#include "gmock/gmock.h"
#include "adcs/adcs.hpp"

struct AdcsCoordinatorMock : adcs::IAdcsCoordinator
{
    MOCK_CONST_METHOD0(CurrentMode, adcs::AdcsMode());

    MOCK_METHOD0(EnableBuiltinDetumbling, OSResult());

    MOCK_METHOD0(EnableCustomDetumbling, OSResult());

    MOCK_METHOD0(EnableSunPointing, OSResult());

    MOCK_METHOD0(Disable, OSResult());
};

struct DetumblingMock : adcs::IDetumblingSupport
{
    MOCK_METHOD0(EnableDetumbling, OSResult());

    MOCK_METHOD0(DisableDetumbling, OSResult());
};

struct SunPointingMock : adcs::ISunPointingSupport
{
    MOCK_METHOD0(EnableSunPointing, OSResult());

    MOCK_METHOD0(DisableSunPointing, OSResult());
};

#endif
