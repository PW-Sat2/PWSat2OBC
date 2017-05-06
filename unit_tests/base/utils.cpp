#include "utils.hpp"

void PrintTo(const Hex& num, ::std::ostream* os)
{
    *os << std::hex << (uint32_t)num;
}
