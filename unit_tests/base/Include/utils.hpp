#ifndef UNIT_TESTS_UTILS_HPP_
#define UNIT_TESTS_UTILS_HPP_

#include <cstdint>
#include <gsl/span>
#include <ostream>
#include <gmock/gmock.h>

#include "gmock/gmock-generated-matchers.h"
#include "utils.h"

class Hex
{
  public:
    explicit Hex(std::uint32_t n) : number(n)
    {
    }
    operator std::uint32_t() const
    {
        return number;
    }

    int operator==(const Hex& other) const
    {
        return this->number == other.number;
    }

  private:
    std::uint32_t number;
};

void PrintTo(const Hex& num, ::std::ostream* os);

template <typename T> void PrintTo(const Option<T>& value, ::std::ostream* os)
{
    if (value.HasValue)
    {
        *os << "Some(" << testing::PrintToString(value.Value) << ")";
    }
    else
    {
        *os << "None";
    }
}

MATCHER_P(DataEqStr, n, "")
{
    return strcmp((const char*)arg, n) == 0;
}

MATCHER_P(SingletonSpan, expected, std::string("Span contains single item " + testing::PrintToString(expected)))
{
    return arg.size() == 1 && arg[0] == expected;
}

MATCHER_P(SpanOfSize, expected, std::string("Span has size " + testing::PrintToString(expected)))
{
    return arg.size() == expected;
}

template <std::size_t Arg> auto FillBuffer(gsl::span<const std::uint8_t> items)
{
    return testing::WithArg<Arg>(
        testing::Invoke([items](gsl::span<std::uint8_t> buffer) { std::copy(items.cbegin(), items.cend(), buffer.begin()); }));
}

template <std::size_t Arg> auto FillBuffer(gsl::span<const std::uint8_t> items1, gsl::span<const std::uint8_t> items2)
{
    return testing::WithArg<Arg>(testing::Invoke([items1, items2](gsl::span<std::uint8_t> buffer) {
        std::copy(items1.cbegin(), items1.cend(), buffer.begin());
        std::copy(items2.cbegin(), items2.cend(), buffer.begin() + items1.length());
    }));
}

template <std::size_t Arg> auto FillBuffer(std::uint8_t value)
{
    return testing::WithArg<Arg>(testing::Invoke([value](gsl::span<std::uint8_t> buffer) { buffer[0] = value; }));
}

MATCHER_P(BeginsWith, value, std::string("begins with " + value))
{
    return arg[0] == value;
}

#endif /* UNIT_TESTS_UTILS_HPP_ */
