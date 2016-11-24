#ifndef UNIT_TESTS_UTILS_HPP_
#define UNIT_TESTS_UTILS_HPP_

#include <cstdint>
#include <gsl/span>
#include <ostream>
#include <gmock/gmock.h>

#include "gmock/gmock-generated-matchers.h"

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

MATCHER_P(DataEqStr, n, "")
{
    return strcmp((const char*)arg, n) == 0;
}

MATCHER_P(BeginsWith, value, std::string("begins with " + value))
{
    return arg[0] == value;
}

#endif /* UNIT_TESTS_UTILS_HPP_ */
