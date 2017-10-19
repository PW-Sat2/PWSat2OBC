#ifndef ADCS_COORDINATOR_MOCK_HPP
#define ADCS_COORDINATOR_MOCK_HPP

#pragma once

#include "gmock/gmock.h"
#include "adcs/adcs.hpp"

struct AdcsCoordinatorMock : adcs::IAdcsCoordinator
{
    AdcsCoordinatorMock();
    ~AdcsCoordinatorMock();

    MOCK_METHOD0(Initialize, OSResult());

    MOCK_CONST_METHOD0(CurrentMode, adcs::AdcsMode());

    MOCK_METHOD0(EnableBuiltinDetumbling, OSResult());

    MOCK_METHOD0(EnableExperimentalDetumbling, OSResult());

    MOCK_METHOD0(EnableSunPointing, OSResult());

    MOCK_METHOD0(Disable, OSResult());

    MOCK_METHOD0(Stop, OSResult());

    MOCK_METHOD2(SetBlockMode, void(adcs::AdcsMode adcsMode, bool isBlocked));

    void SetCurrentMode(adcs::AdcsMode mode);
};

struct DetumblingMock : adcs::IAdcsProcessor
{
    DetumblingMock();
    ~DetumblingMock();

    MOCK_METHOD0(Initialize, OSResult());

    MOCK_METHOD0(Enable, OSResult());

    MOCK_METHOD0(Disable, OSResult());

    MOCK_METHOD0(Process, void());

    MOCK_CONST_METHOD0(GetWait, std::chrono::milliseconds());
};

using SunPointingMock = DetumblingMock;

#endif
