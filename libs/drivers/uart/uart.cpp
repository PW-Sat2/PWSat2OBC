#include "uart.h"
#include <stdio.h>
#include <dmadrv.h>
#include <em_cmu.h>
#include <em_gpio.h>
#include <em_usart.h>
#include "io_map.h"
#include "logger/logger.h"
#include "system.h"

#define DEBUG_UART

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

            this->_event.Initialize();
        }

        void UART::Initialize()
        {
            USART_InitAsync_TypeDef init = USART_INITASYNC_DEFAULT;
            init.baudrate = io_map::UART::Baudrate;
            init.enable = usartDisable;

            CMU_ClockEnable(cmuClock_UART1, true);
            USART_InitAsync(io_map::UART::Peripheral, &init);

            io_map::UART::Peripheral->ROUTE |= UART_ROUTE_TXPEN | UART_ROUTE_RXPEN | io_map::UART::Location;

            USART_IntEnable(io_map::UART::Peripheral, UART_IEN_RXDATAV);

            NVIC_SetPriority(IRQn::UART1_RX_IRQn, io_map::UART::InterruptPriority);
            NVIC_SetPriority(io_map::UART::WakeUpInterrupt, io_map::UART::WakeUpInterruptPriority);
            NVIC_EnableIRQ(io_map::UART::WakeUpInterrupt);

            USART_Enable(io_map::UART::Peripheral, usartEnable);

            DMADRV_AllocateChannel(&this->_rxChannel, nullptr);
            DMADRV_AllocateChannel(&this->_txChannel, nullptr);

#ifdef DEBUG_UART
            GPIO_PinModeSet(gpioPortC, 11, gpioModePushPull, 1);
            GPIO_PinModeSet(gpioPortC, 12, gpioModePushPull, 1);
            GPIO_PinModeSet(gpioPortC, 13, gpioModePushPull, 1);
#endif
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

        size_t UART::Readline(struct _LineIO* io, char* buffer, std::size_t bufferLength, char promptChar)
        {
            auto This = reinterpret_cast<UART*>(io->extra);

            This->_event.Clear(Event::LineEndReceived);
#ifdef DEBUG_UART
            GPIO_PinOutClear(gpioPortC, 12);
#endif
            This->_buffer = buffer;
            This->_bufferEnd = buffer + bufferLength;

            NVIC_EnableIRQ(IRQn::UART1_RX_IRQn);

            if (promptChar != '\0')
            {
                USART_Tx(io_map::UART::Peripheral, promptChar);
            }

            This->_event.WaitAny(Event::LineEndReceived, true, InfiniteTimeout);

            NVIC_DisableIRQ(IRQn::UART1_RX_IRQn);

#ifdef DEBUG_UART
            GPIO_PinOutSet(gpioPortC, 12);
#endif

            *(This->_buffer - 1) = 0;

            return (This->_buffer - buffer);
        }

        void UART::ExchangeBuffers(LineIO* io, gsl::span<const std::uint8_t> outputBuffer, gsl::span<uint8_t> inputBuffer)
        {
            auto This = reinterpret_cast<UART*>(io->extra);

            USART_IntDisable(io_map::UART::Peripheral, UART_IEN_RXDATAV);

            This->_event.Clear(Event::TransferTXFinished | Event::TransferRXFinished);

            DMADRV_PeripheralMemory(This->_rxChannel,
                dmadrvPeripheralSignal_UART1_RXDATAV,
                inputBuffer.data(),
                reinterpret_cast<void*>(const_cast<std::uint32_t*>(&io_map::UART::Peripheral->RXDATA)),
                true,
                inputBuffer.size(),
                dmadrvDataSize1,
                OnDma,
                This);

            DMADRV_MemoryPeripheral(This->_txChannel,
                dmadrvPeripheralSignal_UART1_TXBL,
                reinterpret_cast<void*>(const_cast<std::uint32_t*>(&io_map::UART::Peripheral->TXDATA)),
                const_cast<std::uint8_t*>(outputBuffer.data()),
                true,
                outputBuffer.size(),
                dmadrvDataSize1,
                OnDma,
                This);

            This->_event.WaitAll(Event::TransferTXFinished | Event::TransferRXFinished, true, InfiniteTimeout);

            USART_IntEnable(io_map::UART::Peripheral, UART_IEN_RXDATAV);
        }

        __attribute__((optimize("O3"))) void UART::OnReceived()
        {
            auto b = io_map::UART::Peripheral->RXDATA & 0x0FF;
            *this->_buffer = b;
            this->_buffer++;
            if (b == '\n' || this->_buffer == this->_bufferEnd)
            {
#ifdef DEBUG_UART
                GPIO_PinOutClear(gpioPortC, 13);
#endif

                NVIC_SetPendingIRQ(io_map::UART::WakeUpInterrupt);
            }
#ifdef DEBUG_UART
            GPIO->P[gpioPortC].DOUTSET = 1 << 11;
#endif
        }

        void UART::OnWakeUpInterrupt()
        {
#ifdef DEBUG_UART
            GPIO_PinOutSet(gpioPortC, 13);
#endif
            this->_event.SetISR(Event::LineEndReceived);
        }

        bool UART::OnDma(unsigned int channel, unsigned int /*sequenceNo*/, void* userParam)
        {
            auto This = reinterpret_cast<UART*>(userParam);

            if (channel == This->_rxChannel)
            {
                This->_event.SetISR(Event::TransferRXFinished);
            }
            else if (channel == This->_txChannel)
            {
                This->_event.SetISR(Event::TransferTXFinished);
            }

            return true;
        }
    }
}
