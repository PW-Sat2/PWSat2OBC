#ifndef UNIT_TESTS_MOCK_COMM_HPP_
#define UNIT_TESTS_MOCK_COMM_HPP_

#include <cstdint>
#include "gmock/gmock.h"
#include "comm/ITransmitFrame.hpp"
#include "gsl/span"

struct TransmitFrameMock : public devices::comm::ITransmitFrame
{
    MOCK_METHOD1(SendFrame, bool(gsl::span<const std::uint8_t>));
};

#endif /* UNIT_TESTS_MOCK_COMM_HPP_ */
