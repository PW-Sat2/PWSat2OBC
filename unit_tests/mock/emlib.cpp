#include "emlib.h"

static ICMU* cmuProxy = nullptr;
static IUSART* usartProxy = nullptr;
static IDMA* dmaProxy = nullptr;

namespace efm
{
    namespace cmu
    {
        void ClockEnable(CMU_Clock_TypeDef clock, bool enable)
        {
            if (cmuProxy != nullptr)
            {
                cmuProxy->ClockEnable(clock, enable);
            }
        }
    }

    namespace usart
    {
        void InitSync(USART_TypeDef* usart, const USART_InitSync_TypeDef* init)
        {
            if (usartProxy != nullptr)
            {
                usartProxy->InitSync(usart, init);
            }
        }

        void Enable(USART_TypeDef* usart, USART_Enable_TypeDef enable)
        {
            if (usartProxy != nullptr)
            {
                usartProxy->Enable(usart, enable);
            }
        }

        void IntClear(USART_TypeDef* usart, uint32_t flags)
        {
            if (usartProxy != nullptr)
            {
                usartProxy->IntClear(usart, flags);
            }
        }

        uint32_t IntGet(USART_TypeDef* usart)
        {
            if (usartProxy != nullptr)
            {
                return usartProxy->IntGet(usart);
            }

            return 0ul;
        }

        void AmendRoute(USART_TypeDef* usart, uint32_t flags)
        {
            if (usartProxy != nullptr)
            {
                usartProxy->AmendRoute(usart, flags);
            }
        }

        void Command(USART_TypeDef* usart, uint32_t cmd)
        {
            if (usartProxy != nullptr)
            {
                usartProxy->Command(usart, cmd);
            }
        }
    }

    namespace dma
    {
        Ecode_t AllocateChannel(unsigned int* channelId, void* capabilities)
        {
            if (dmaProxy != nullptr)
            {
                return dmaProxy->AllocateChannel(channelId, capabilities);
            }

            return ECODE_EMDRV_DMADRV_NOT_INITIALIZED;
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
            if (dmaProxy != nullptr)
            {
                return dmaProxy->PeripheralMemory(channelId, peripheralSignal, dst, src, dstInc, len, size, callback, cbUserParam);
            }

            return ECODE_EMDRV_DMADRV_NOT_INITIALIZED;
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
            if (dmaProxy != nullptr)
            {
                return dmaProxy->MemoryPeripheral(channelId, peripheralSignal, dst, src, srcInc, len, size, callback, cbUserParam);
            }

            return ECODE_EMDRV_DMADRV_NOT_INITIALIZED;
        }
    }
}

CMUReset InstallProxy(ICMU* target)
{
    cmuProxy = target;
    return CMUReset();
}

USARTReset InstallProxy(IUSART* target)
{
    usartProxy = target;
    return USARTReset();
}

DMAReset InstallProxy(IDMA* target)
{
    dmaProxy = target;
    return DMAReset();
}

CMUReset::CMUReset() : ProxyReset<ICMU>(cmuProxy)
{
}

USARTReset::USARTReset() : ProxyReset<IUSART>(usartProxy)
{
}

DMAReset::DMAReset() : ProxyReset<IDMA>(dmaProxy)
{
}
