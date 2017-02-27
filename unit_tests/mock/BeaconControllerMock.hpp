#ifndef BEACON_CONTROLLER_MOCK_HPP
#define BEACON_CONTROLLER_MOCK_HPP

#pragma once

#include "gmock/gmock.h"
#include "comm/IBeaconController.hpp"

struct BeaconControllerMock : public devices::comm::IBeaconController
{
    MOCK_METHOD1(SetBeacon, bool(const devices::comm::Beacon& beacon));
    MOCK_METHOD0(ClearBeacon, bool());
};

#endif
