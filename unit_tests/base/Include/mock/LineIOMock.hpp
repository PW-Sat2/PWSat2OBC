#ifndef UNIT_TESTS_BASE_INCLUDE_MOCK_LINEIOMOCK_HPP_
#define UNIT_TESTS_BASE_INCLUDE_MOCK_LINEIOMOCK_HPP_

#include <stdarg.h>
#include <chrono>
#include <cstdint>
#include <gsl/span>
#include "line_io.h"

struct LineIOMock : public ILineIO
{
    LineIOMock();
    ~LineIOMock();

    MOCK_METHOD1(Puts, void(const char* s));

    MOCK_METHOD2(VPrintf, void(const char* text, va_list args));

    MOCK_METHOD1(PrintBuffer, void(gsl::span<const std::uint8_t> buffer));

    MOCK_METHOD3(Readline, size_t(char* buffer, size_t bufferLength, char promptChar));

    MOCK_METHOD2(Read, bool(gsl::span<std::uint8_t> buffer, std::chrono::milliseconds timeout));

    MOCK_METHOD3(ExchangeBuffers,
        bool(gsl::span<const std::uint8_t> outputBuffer, gsl::span<std::uint8_t> inputBuffer, std::chrono::milliseconds timeout));
};

#endif // UNIT_TESTS_BASE_INCLUDE_MOCK_LINEIOMOCK_HPP_
