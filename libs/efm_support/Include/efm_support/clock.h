#ifndef LIBS_EFM_SUPPORT_INCLUDE_EFM_SUPPORT_CLOCK_H_
#define LIBS_EFM_SUPPORT_INCLUDE_EFM_SUPPORT_CLOCK_H_

#include <em_cmu.h>

namespace efm
{
    template <typename Peripheral> constexpr CMU_Clock_TypeDef Clock(const Peripheral hw)
    {
        if (hw == USART1)
            return cmuClock_USART1;

        return static_cast<CMU_Clock_TypeDef>(0);
    }
}

#endif /* LIBS_EFM_SUPPORT_INCLUDE_EFM_SUPPORT_CLOCK_H_ */
