#ifndef UNIT_TESTS_MOCK_ERROR_COUNTER_HPP_
#define UNIT_TESTS_MOCK_ERROR_COUNTER_HPP_

#include "gmock/gmock.h"
#include "error_counter/error_counter.hpp"

struct ErrorCountingConfigrationMock : public error_counter::IErrorCountingConfigration
{
    ErrorCountingConfigrationMock();

    MOCK_METHOD1(Limit, error_counter::CounterValue(error_counter::Device device));
    MOCK_METHOD1(Increment, error_counter::CounterValue(error_counter::Device device));
    MOCK_METHOD1(Decrement, error_counter::CounterValue(error_counter::Device device));
};

namespace error_counter
{
    template <Device Device> bool operator==(const ErrorCounter<Device> lhs, const int rhs)
    {
        return lhs.Current() == rhs;
    }

    template <Device Device> void PrintTo(const ErrorCounter<Device>& value, ::std::ostream* os)
    {
        *os << "Errors=" << static_cast<std::uint16_t>(value.Current());
    }
}

#endif /* UNIT_TESTS_MOCK_ERROR_COUNTER_HPP_ */
