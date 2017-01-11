#ifndef UNIT_TESTS_EMLIB_EMLIB_H_
#define UNIT_TESTS_EMLIB_EMLIB_H_

#include <dmadrv.h>
#include <em_cmu.h>
#include <em_usart.h>
#include <utility>
#include "gmock/gmock.h"

struct ICMU
{
    virtual void ClockEnable(CMU_Clock_TypeDef clock, bool enable) = 0;
};

struct CMUMock : public ICMU
{
    MOCK_METHOD2(ClockEnable, void(CMU_Clock_TypeDef clock, bool enable));
};

struct IUSART
{
    virtual void InitSync(USART_TypeDef* usart, const USART_InitSync_TypeDef* init) = 0;

    virtual void Enable(USART_TypeDef* usart, USART_Enable_TypeDef enable) = 0;

    virtual void IntClear(USART_TypeDef* usart, uint32_t flags) = 0;

    virtual uint32_t IntGet(USART_TypeDef* usart) = 0;

    virtual void AmendRoute(USART_TypeDef* usart, uint32_t flags) = 0;

    virtual void Command(USART_TypeDef* usart, uint32_t cmd) = 0;
};

struct USARTMock : public IUSART
{
    MOCK_METHOD2(InitSync, void(USART_TypeDef* usart, const USART_InitSync_TypeDef* init));

    MOCK_METHOD2(Enable, void(USART_TypeDef* usart, USART_Enable_TypeDef enable));

    MOCK_METHOD2(IntClear, void(USART_TypeDef* usart, uint32_t flags));

    MOCK_METHOD1(IntGet, uint32_t(USART_TypeDef* usart));

    MOCK_METHOD2(AmendRoute, void(USART_TypeDef* usart, uint32_t flags));

    MOCK_METHOD2(Command, void(USART_TypeDef* usart, uint32_t cmd));
};

struct IDMA
{
    virtual Ecode_t AllocateChannel(unsigned int* channelId, void* capabilities) = 0;

    virtual Ecode_t PeripheralMemory(unsigned int channelId,
        DMADRV_PeripheralSignal_t peripheralSignal,
        void* dst,
        void* src,
        bool dstInc,
        int len,
        DMADRV_DataSize_t size,
        DMADRV_Callback_t callback,
        void* cbUserParam) = 0;

    virtual Ecode_t MemoryPeripheral(unsigned int channelId,
        DMADRV_PeripheralSignal_t peripheralSignal,
        void* dst,
        void* src,
        bool srcInc,
        int len,
        DMADRV_DataSize_t size,
        DMADRV_Callback_t callback,
        void* cbUserParam) = 0;
};

struct DMAMock : public IDMA
{
    MOCK_METHOD2(AllocateChannel, Ecode_t(unsigned int* channelId, void* capabilities));

    MOCK_METHOD9(PeripheralMemory,
        Ecode_t(unsigned int channelId,
            DMADRV_PeripheralSignal_t peripheralSignal,
            void* dst,
            void* src,
            bool dstInc,
            int len,
            DMADRV_DataSize_t size,
            DMADRV_Callback_t callback,
            void* cbUserParam));

    MOCK_METHOD9(MemoryPeripheral,
        Ecode_t(unsigned int channelId,
            DMADRV_PeripheralSignal_t peripheralSignal,
            void* dst,
            void* src,
            bool srcInc,
            int len,
            DMADRV_DataSize_t size,
            DMADRV_Callback_t callback,
            void* cbUserParam));
};

template <typename Mock> class ProxyReset
{
  public:
    using ProxyHolderPtr = Mock*;

    ProxyReset(ProxyHolderPtr& proxyHolder);
    ~ProxyReset();

    ProxyReset(ProxyReset<Mock>&& arg) noexcept;
    ProxyReset<Mock>& operator=(ProxyReset<Mock>&& arg) noexcept;

  private:
    ProxyHolderPtr& _holder;
    bool _released;
};

template <typename Mock> ProxyReset<Mock>::ProxyReset(ProxyHolderPtr& proxyHolder) : _holder(proxyHolder), _released(false)
{
}

template <typename Mock> ProxyReset<Mock>::~ProxyReset()
{
    if (!this->_released)
    {
        this->_holder = nullptr;
    }
}

template <typename Mock> ProxyReset<Mock>::ProxyReset(ProxyReset<Mock>&& arg) noexcept : _holder(arg._holder), _released(arg._released)
{
    arg._released = true;
}

template <typename Mock> ProxyReset<Mock>& ProxyReset<Mock>::operator=(ProxyReset<Mock>&& arg) noexcept
{
    ProxyReset<Mock> tmp(std::move(arg));

    std::swap(this->_released, tmp._released);
    std::swap(this->_holder, tmp._holder);

    tmp._released = true;

    return *this;
}

class CMUReset : public ProxyReset<ICMU>
{
  public:
    CMUReset();
};

class USARTReset : public ProxyReset<IUSART>
{
  public:
    USARTReset();
};

class DMAReset : public ProxyReset<IDMA>
{
  public:
    DMAReset();
};

CMUReset InstallProxy(ICMU* target);
USARTReset InstallProxy(IUSART* target);
DMAReset InstallProxy(IDMA* target);

#endif /* UNIT_TESTS_EMLIB_EMLIB_H_ */
