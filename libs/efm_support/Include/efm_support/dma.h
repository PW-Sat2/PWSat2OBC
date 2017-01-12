#ifndef LIBS_EFM_SUPPORT_INCLUDE_EFM_SUPPORT_DMA_H_
#define LIBS_EFM_SUPPORT_INCLUDE_EFM_SUPPORT_DMA_H_

#include <cstdint>
#include <dmadrv.h>
#include <em_device.h>

namespace efm
{
    /**
     * @brief Possible DMA signals for USART peripheral
     * @ingroup efm_support
     */
    enum class DMASignalUSART
    {
        RXDATAV, //!< RXDATAV
        TXBL     //!< TXBL
    };

    /**
     * @brief Returns specific DMA signal for given USART peripheral
     * @param peripheral Peripheral base address
     * @return DMA signal
     * @ingroup efm_support
     */
    template <DMASignalUSART Signal> constexpr DMADRV_PeripheralSignal_t DMASignal(const USART_TypeDef* peripheral) = delete;

    template <> constexpr DMADRV_PeripheralSignal_t DMASignal<DMASignalUSART::RXDATAV>(const USART_TypeDef* peripheral)
    {
        if (peripheral == USART1)
            return dmadrvPeripheralSignal_USART1_RXDATAV;

        return static_cast<DMADRV_PeripheralSignal_t>(-1);
    }

    template <> constexpr DMADRV_PeripheralSignal_t DMASignal<DMASignalUSART::TXBL>(const USART_TypeDef* peripheral)
    {
        if (peripheral == USART1)
            return dmadrvPeripheralSignal_USART1_TXBL;

        return static_cast<DMADRV_PeripheralSignal_t>(-1);
    }
}

#endif /* LIBS_EFM_SUPPORT_INCLUDE_EFM_SUPPORT_DMA_H_ */
