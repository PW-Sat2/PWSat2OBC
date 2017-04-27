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

__attribute__((optimize("O3"))) void UART1_RX_IRQHandler()
{
#ifdef DEBUG_UART
    GPIO->P[gpioPortC].DOUTCLR = 1 << 11;
//    GPIO_PinOutClear(gpioPortC, 11);
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

            this->_event.Initialize();

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
            NVIC_SetPriority(io_map::UART::WakeUpInterrupt, io_map::UART::WakeUpInterruptPriority);
            NVIC_EnableIRQ(IRQn::UART1_RX_IRQn);
            NVIC_EnableIRQ(io_map::UART::WakeUpInterrupt);

            USART_Enable(io_map::UART::Peripheral, usartEnable);

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

        size_t UART::Readline(struct _LineIO* io, char* buffer, std::size_t /*bufferLength*/)
        {
            auto This = reinterpret_cast<UART*>(io->extra);

            This->_event.Clear(Event::LineEndReceived);

#ifdef DEBUG_UART
            GPIO_PinOutClear(gpioPortC, 12);
#endif
            //            DMADRV_PeripheralMemory(This->_rxChannel,
            //                dmadrvPeripheralSignal_UART1_RXDATAV,
            //                buffer,
            //                const_cast<void*>(reinterpret_cast<volatile void*>(&io_map::UART::Peripheral->RXDATA)),
            //                true,
            //                bufferLength,
            //                dmadrvDataSize1,
            //                OnDma,
            //                This);

            This->_buffer = buffer;

            This->_event.WaitAny(Event::LineEndReceived, true, InfiniteTimeout);

            //            int remaining = 0;
            //            DMADRV_TransferRemainingCount(This->_rxChannel, &remaining);

            LOGF(LOG_LEVEL_DEBUG, "Buffer: '%s' (%d %d %d %d)", buffer, buffer[0], buffer[1], buffer[2], buffer[3]);

            //            if ((size_t)remaining == bufferLength)
            //            {
            //#ifdef DEBUG_UART
            //                GPIO_PinOutSet(gpioPortC, 12);
            //#endif
            //
            //                buffer[0] = 0;
            //                return 0;
            //            }
            //
            //            if (buffer[bufferLength - remaining - 1] == '\n')
            //            {
            //                buffer[bufferLength - remaining - 1] = 0;
            //            }
            //
            //            else if (buffer[bufferLength - remaining - 1] != '\n')
            //            {
            //                buffer[bufferLength - remaining] = 0;
            //                remaining--;
            //            }

            *(This->_buffer - 1) = 0;

//            while (1)
//            {
//
//                if (static_cast<std::size_t>(remaining) == bufferLength)
//                    continue;
//
//                volatile char* c = &buffer[bufferLength - remaining - 1];
//
//                if (*c == '\n')
//                    break;
//            }
#ifdef DEBUG_UART
            GPIO_PinOutSet(gpioPortC, 12);
#endif

            //            DMADRV_StopTransfer(This->_rxChannel);
            //            LOGF(LOG_LEVEL_INFO, "Received line end. R=%d", remaining);

            //            buffer[bufferLength - remaining - 1] = '\0';

            return (This->_buffer - buffer);

            //            while (1)
            //                buffer[0] = 0;
            //
            //            char* place = buffer;
            //
            //            while (true)
            //            {
            //#ifdef DEBUG_UART
            //                GPIO_PinOutClear(gpioPortC, 12);
            //#endif
            //
            //                *place = USART_Rx(io_map::UART::Peripheral);
            //
            //#ifdef DEBUG_UART
            //                GPIO_PinOutSet(gpioPortC, 12);
            //#endif
            //
            //                if (*place == '\n')
            //                {
            //                    *place = 0;
            //                    break;
            //                }
            //                place++;
            //            }
            //
            //            return place - buffer;
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

        __attribute__((optimize("O3"))) void UART::OnReceived()
        {
            auto b = io_map::UART::Peripheral->RXDATA & 0x0FF;
            *this->_buffer = b;
            this->_buffer++;
            if (b == '\n')
            {
                //                            System::GiveSemaphoreISR(this->_receivedLineEnd);
                //                            System::EndSwitchingISR();
                //                DMADRV_StopTransfer(this->_rxChannel);
                //                auto x = ((DMA_DESCRIPTOR_TypeDef*)(DMA->CTRLBASE)) + this->_rxChannel;
                //                x->CTRL &= ~(_DMA_CTRL_N_MINUS_1_MASK);

                //                DMA->CHENC = 1 << this->_rxChannel;
                //                DMA->IFS = 1 << this->_rxChannel;
                //                NVIC_SetPendingIRQ(IRQn_Type::DMA_IRQn);

                NVIC_SetPendingIRQ(io_map::UART::WakeUpInterrupt);
            }

            GPIO->P[gpioPortC].DOUTSET = 1 << 11;
        }

        void UART::OnWakeUpInterrupt()
        {
            this->_event.SetISR(Event::LineEndReceived);
        }

        bool UART::OnDma(unsigned int /*channel*/, unsigned int /*sequenceNo*/, void* userParam)
        {
            GPIO->P[gpioPortC].DOUTCLR = 1 << 13;

            auto This = reinterpret_cast<UART*>(userParam);

            This->_event.SetISR(Event::LineEndReceived);

            GPIO->P[gpioPortC].DOUTSET = 1 << 13;

            return true;
        }
    }
}
