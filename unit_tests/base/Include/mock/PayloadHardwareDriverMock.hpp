#ifndef UNIT_TESTS_BASE_INCLUDE_MOCK_PAYLOADHARDWAREDRIVERMOCK_HPP_
#define UNIT_TESTS_BASE_INCLUDE_MOCK_PAYLOADHARDWAREDRIVERMOCK_HPP_

#pragma once

#include "gmock/gmock.h"
#include "payload/payload.h"

class PayloadHardwareDriverMock : public devices::payload::IPayloadDriver
{
  public:
    PayloadHardwareDriverMock();

    MOCK_CONST_METHOD0(IsBusy, bool());

    MOCK_METHOD2(PayloadRead, OSResult(gsl::span<std::uint8_t>, gsl::span<std::uint8_t>));

    MOCK_METHOD1(PayloadWrite, OSResult(gsl::span<std::uint8_t>));

    MOCK_METHOD0(WaitForData, OSResult());

    MOCK_METHOD0(RaiseDataReadyISR, void());

    MOCK_METHOD1(SetDataTimeout, void(std::chrono::milliseconds));
};

#endif /* UNIT_TESTS_BASE_INCLUDE_MOCK_PAYLOADHARDWAREDRIVERMOCK_HPP_ */
