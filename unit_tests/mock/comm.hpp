#ifndef UNIT_TESTS_MOCK_COMM_HPP_
#define UNIT_TESTS_MOCK_COMM_HPP_

#include <cstdint>
#include "gmock/gmock.h"
#include "comm/Beacon.hpp"
#include "comm/IBeaconController.hpp"
#include "comm/ITransmitFrame.hpp"
#include "gsl/span"

struct TransmitFrameMock : public devices::comm::ITransmitFrame
{
    TransmitFrameMock();
    MOCK_METHOD1(SendFrame, bool(gsl::span<const std::uint8_t>));
};

struct BeaconControllerMock : public devices::comm::IBeaconController
{
    BeaconControllerMock();
    MOCK_METHOD1(SetBeacon, Option<bool>(const devices::comm::Beacon& beacon));
    MOCK_METHOD0(ClearBeacon, bool());
};

#endif /* UNIT_TESTS_MOCK_COMM_HPP_ */
