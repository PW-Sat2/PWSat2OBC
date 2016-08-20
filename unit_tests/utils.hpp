#ifndef UNIT_TESTS_UTILS_HPP_
#define UNIT_TESTS_UTILS_HPP_

#include <stdint.h>
#include <ostream>

class Hex
{
  public:
    explicit Hex(uint32_t n) : number(n)
    {
    }
    operator uint32_t() const
    {
        return number;
    }

    int operator==(const Hex& other) const
    {
        return this->number == other.number;
    }

  private:
    uint32_t number;
};

void PrintTo(const Hex& num, ::std::ostream* os);

#endif /* UNIT_TESTS_UTILS_HPP_ */
