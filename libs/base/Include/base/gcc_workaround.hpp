#ifndef LIBS_BASE_INCLUDE_BASE_GCC_WORKAROUND_HPP_
#define LIBS_BASE_INCLUDE_BASE_GCC_WORKAROUND_HPP_

#include <cstdint>

namespace gcc_workaround
{
    static inline std::uint8_t ReadByte0()
    {
        std::uint8_t result;

        asm volatile("mov r0, 0\n"
                     "ldr %0, [r0]"
                     : "=r"(result)
                     :
                     : "r0");

        return 0;
    }
}

#endif /* LIBS_BASE_INCLUDE_BASE_GCC_WORKAROUND_HPP_ */
