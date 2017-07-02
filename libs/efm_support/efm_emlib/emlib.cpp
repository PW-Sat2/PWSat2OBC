#include <em_device.h>
#include <em_rmu.h>
#include "efm_support/api.h"

namespace efm
{
    namespace cmu
    {
        void ClockEnable(CMU_Clock_TypeDef clock, bool enable)
        {
            CMU_ClockEnable(clock, enable);
        }
    }

    namespace usart
    {
        void InitSync(USART_TypeDef* usart, const USART_InitSync_TypeDef* init)
        {
            USART_InitSync(usart, init);
        }

        void Enable(USART_TypeDef* usart, USART_Enable_TypeDef enable)
        {
            USART_Enable(usart, enable);
        }

        void IntClear(USART_TypeDef* usart, uint32_t flags)
        {
            USART_IntClear(usart, flags);
        }

        uint32_t IntGet(USART_TypeDef* usart)
        {
            return USART_IntGet(usart);
        }

        void AmendRoute(USART_TypeDef* usart, uint32_t flags)
        {
            usart->ROUTE |= flags;
        }

        void Command(USART_TypeDef* usart, uint32_t cmd)
        {
            usart->CMD = cmd;
        }
    }

    namespace dma
    {
        Ecode_t AllocateChannel(unsigned int* channelId, void* capabilities)
        {
            return DMADRV_AllocateChannel(channelId, capabilities);
        }

        Ecode_t PeripheralMemory(unsigned int channelId,
            DMADRV_PeripheralSignal_t peripheralSignal,
            void* dst,
            void* src,
            bool dstInc,
            int len,
            DMADRV_DataSize_t size,
            DMADRV_Callback_t callback,
            void* cbUserParam)
        {
            return DMADRV_PeripheralMemory(channelId, peripheralSignal, dst, src, dstInc, len, size, callback, cbUserParam);
        }

        Ecode_t MemoryPeripheral(unsigned int channelId,
            DMADRV_PeripheralSignal_t peripheralSignal,
            void* dst,
            void* src,
            bool srcInc,
            int len,
            DMADRV_DataSize_t size,
            DMADRV_Callback_t callback,
            void* cbUserParam)
        {
            return DMADRV_MemoryPeripheral(channelId, peripheralSignal, dst, src, srcInc, len, size, callback, cbUserParam);
        }
    }

    namespace mcu
    {
        std::uint32_t GetBootReason()
        {
            return RMU->RSTCAUSE;
        }

        void ResetBootReason()
        {
            RMU_ResetCauseClear();
        }
    }
}
