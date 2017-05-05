#ifndef UNIT_TESTS_MOCK_COMM_HPP_
#define UNIT_TESTS_MOCK_COMM_HPP_

#include <cstdint>
#include "gmock/gmock.h"
#include "comm/Beacon.hpp"
#include "comm/IBeaconController.hpp"
#include "comm/ITransmitter.hpp"
#include "gsl/span"

struct TransmitterMock : public devices::comm::ITransmitter
{
    TransmitterMock();
    MOCK_METHOD1(SetTransmitterStateWhenIdle, bool(devices::comm::IdleState));
    MOCK_METHOD1(SendFrame, bool(gsl::span<const std::uint8_t>));
};

struct BeaconControllerMock : public devices::comm::IBeaconController
{
    BeaconControllerMock();
    MOCK_METHOD1(SetBeacon, Option<bool>(const devices::comm::Beacon& beacon));
    MOCK_METHOD0(ClearBeacon, bool());
};

#endif /* UNIT_TESTS_MOCK_COMM_HPP_ */
