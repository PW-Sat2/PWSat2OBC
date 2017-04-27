#ifndef LIBS_EFM_SUPPORT_INCLUDE_EFM_SUPPORT_CLOCK_H_
#define LIBS_EFM_SUPPORT_INCLUDE_EFM_SUPPORT_CLOCK_H_

#include <em_cmu.h>

namespace efm
{
    /**
     * @brief Return CMU clock associated with given periperhal
     * @param hw Peripheral base address
     * @return CMU clock
     *
     * @remark Not all peripherals are supported at the moment - feel free to add new ones if needed
     *
     * @ingroup efm_support
     */
    template <typename Peripheral> constexpr CMU_Clock_TypeDef Clock(const Peripheral hw)
    {
        if (hw == USART0)
            return cmuClock_USART0;

        if (hw == USART1)
            return cmuClock_USART1;

        return static_cast<CMU_Clock_TypeDef>(0);
    }
}

#endif /* LIBS_EFM_SUPPORT_INCLUDE_EFM_SUPPORT_CLOCK_H_ */
