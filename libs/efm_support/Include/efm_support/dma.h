#ifndef LIBS_EFM_SUPPORT_INCLUDE_EFM_SUPPORT_DMA_H_
#define LIBS_EFM_SUPPORT_INCLUDE_EFM_SUPPORT_DMA_H_

#include <cstdint>
#include <dmadrv.h>
#include <em_device.h>

namespace efm
{
    enum class DMASignalUSART
    {
        RXDATAV,
        TXBL
    };

    template <DMASignalUSART Signal> constexpr DMADRV_PeripheralSignal_t DMASignal(USART_TypeDef* peripheral) = delete;

    template <> constexpr DMADRV_PeripheralSignal_t DMASignal<DMASignalUSART::RXDATAV>(USART_TypeDef* peripheral)
    {
        if (peripheral == USART1)
            return dmadrvPeripheralSignal_USART1_RXDATAV;

        return static_cast<DMADRV_PeripheralSignal_t>(-1);
    }

    template <> constexpr DMADRV_PeripheralSignal_t DMASignal<DMASignalUSART::TXBL>(USART_TypeDef* peripheral)
    {
        if (peripheral == USART1)
            return dmadrvPeripheralSignal_USART1_TXBL;

        return static_cast<DMADRV_PeripheralSignal_t>(-1);
    }
}

#endif /* LIBS_EFM_SUPPORT_INCLUDE_EFM_SUPPORT_DMA_H_ */
