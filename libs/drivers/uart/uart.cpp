#include "uart.h"
#include <stdio.h>
#include <dmadrv.h>
#include <em_cmu.h>
#include <em_gpio.h>
#include <em_usart.h>
#include "io_map.h"
#include "logger/logger.h"
#include "system.h"

static drivers::uart::UART* uart = nullptr;

#define DEBUG_UART

void UART1_RX_IRQHandler()
{
#ifdef DEBUG_UART
    GPIO_PinOutClear(gpioPortC, 11);
#endif

    if (uart != nullptr)
    {
        uart->OnReceived();
    }
}

namespace drivers
{
    namespace uart
    {
        UART::UART()
        {
            this->_lineIO.extra = this;
            this->_lineIO.PrintBuffer = PrintBuffer;
            this->_lineIO.Puts = Puts;
            this->_lineIO.ExchangeBuffers = ExchangeBuffers;
            this->_lineIO.Readline = Readline;
            this->_lineIO.VPrintf = VPrintf;

            this->_receivedLineEnd = System::CreateBinarySemaphore(0);
            System::GiveSemaphore(this->_receivedLineEnd);

            ::uart = this;
        }

        void UART::Initialize()
        {
            this->_queue.Create();

            USART_InitAsync_TypeDef init = USART_INITASYNC_DEFAULT;
            init.baudrate = io_map::UART::Baudrate;
            init.enable = usartDisable;

            CMU_ClockEnable(cmuClock_UART1, true);
            USART_InitAsync(io_map::UART::Peripheral, &init);

            io_map::UART::Peripheral->ROUTE |= UART_ROUTE_TXPEN | UART_ROUTE_RXPEN | io_map::UART::Location;

            USART_IntEnable(io_map::UART::Peripheral, UART_IEN_RXDATAV);

            NVIC_SetPriority(IRQn::UART1_RX_IRQn, io_map::UART::InterruptPriority);
            NVIC_EnableIRQ(IRQn::UART1_RX_IRQn);

            USART_Enable(io_map::UART::Peripheral, usartEnable);

#ifdef DEBUG_UART
            GPIO_PinModeSet(gpioPortC, 11, gpioModePushPull, 1);
            GPIO_PinModeSet(gpioPortC, 12, gpioModePushPull, 1);
#endif

            DMADRV_AllocateChannel(&this->_rxChannel, nullptr);
        }

        LineIO& UART::GetLineIO()
        {
            return this->_lineIO;
        }
        void UART::Puts(struct _LineIO* io, const char* s)
        {
            UNREFERENCED_PARAMETER(io);

            while (*s != '\0')
            {
                USART_Tx(io_map::UART::Peripheral, *s);
                s++;
            }
        }

        void UART::VPrintf(struct _LineIO* io, const char* text, va_list args)
        {
            char buf[255] = {0};

            vsniprintf(buf, sizeof(buf), text, args);

            UART::Puts(io, buf);
        }

        void UART::PrintBuffer(gsl::span<const std::uint8_t> buffer)
        {
            for (const auto b : buffer)
            {
                USART_Tx(io_map::UART::Peripheral, b);
            }
        }

        size_t UART::Readline(struct _LineIO* io, char* buffer, std::size_t bufferLength)
        {
            auto This = reinterpret_cast<UART*>(io->extra);

#ifdef DEBUG_UART
            GPIO_PinOutClear(gpioPortC, 12);
#endif
            System::TakeSemaphore(This->_receivedLineEnd, InfiniteTimeout);

            DMADRV_PeripheralMemory(This->_rxChannel,
                dmadrvPeripheralSignal_UART1_RXDATAV,
                buffer,
                const_cast<void*>(reinterpret_cast<volatile void*>(&io_map::UART::Peripheral->RXDATA)),
                true,
                bufferLength,
                dmadrvDataSize1,
                nullptr,
                nullptr);

            System::TakeSemaphore(This->_receivedLineEnd, InfiniteTimeout);

            DMADRV_StopTransfer(This->_rxChannel);

            int remaining = 0;

            auto x = DMADRV_TransferRemainingCount(This->_rxChannel, &remaining);

            LOGF(LOG_LEVEL_INFO, "Received line end. R=%d X=%lX", remaining, x);

            buffer[bufferLength - remaining - 1] = '\0';

            System::GiveSemaphore(This->_receivedLineEnd);

#ifdef DEBUG_UART
            GPIO_PinOutSet(gpioPortC, 12);
#endif
            return bufferLength - remaining;
        }

        void UART::ExchangeBuffers(LineIO* io, gsl::span<const std::uint8_t> outputBuffer, gsl::span<uint8_t> inputBuffer)
        {
            UNREFERENCED_PARAMETER(io);

            for (auto& b : outputBuffer)
            {
                USART_Tx(io_map::UART::Peripheral, b);
            }

            USART_IntDisable(io_map::UART::Peripheral, UART_IEN_RXDATAV);

            for (auto& b : inputBuffer)
            {
                b = USART_Rx(io_map::UART::Peripheral);
            }

            USART_IntClear(io_map::UART::Peripheral, UART_IF_RXDATAV);
            USART_IntEnable(io_map::UART::Peripheral, UART_IEN_RXDATAV);
        }

        void UART::OnReceived()
        {
            auto b = USART_RxDataGet(io_map::UART::Peripheral);

            if (b == '\n')
            {
                System::GiveSemaphoreISR(this->_receivedLineEnd);
                System::EndSwitchingISR();
            }

            GPIO_PinOutSet(gpioPortC, 11);
        }
    }
}
