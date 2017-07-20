#ifndef LIBS_DRIVERS_UART_INCLUDE_UART_UART_H_
#define LIBS_DRIVERS_UART_INCLUDE_UART_UART_H_

#include <stdarg.h>
#include <stdio.h>
#include <cstdint>
#include <dmadrv.h>
#include <em_cmu.h>
#include <em_gpio.h>
#include <em_usart.h>
#include <gsl/span>
#include "base/os.h"
#include "line_io.h"
#include "logger/logger.h"
#include "mcu/io_map.h"
#include "system.h"

namespace drivers
{
    namespace uart
    {
        /**
         * @defgroup uart_driver UART Driver
         * @ingroup peripheral_drivers
         */

        /**
         * @brief UART driver
         * @ingroup uart_driver
         */
        template <typename UartPort> class UART
        {
          public:
            /**
             * @brief Ctor
             */
            UART();
            /**
             * @brief Initializes peripheral
             */
            void Initialize();

            /**
             * @brief Returns reference to LineIO implementation
             * @return Line IO implementation
             */
            LineIO& GetLineIO();

            /**
             * @brief Call from UARTx_RX_IRQHandler
             */
            void OnReceived();
            /**
             * @brief Call from wake-up interrupt handler
             */
            void OnWakeUpInterrupt();

            /**
             * @brief Enable basic line editing for UART readline
             * @param enable true if line editing is enabled, false otherwise
             */
            inline void LineEditing(bool enable);

            /**
             * @brief Writes buffer to UART port using DMA tranfer
             * @param buffer Buffer to write
             */
            void WriteBuffer(gsl::span<const std::uint8_t> buffer);

            /**
             * @brief Reads date from UART port into buffer using DMA transfer
             * @param buffer Buffer that will hold upcoming data
             */
            void ReadBuffer(gsl::span<const std::uint8_t> buffer);

          private:
            /**
             * @brief Procedure that outputs given string
             * @param io @see LineIO structure
             * @param s Text to output
             */
            static void Puts(struct _LineIO* io, const char* s);
            /**
             * @brief Procedure that outputs formatted string
             * @param io @see LineIO structure
             * @param text Format string
             * @param args Format atrguments as @see a_list
             */
            static void VPrintf(struct _LineIO* io, const char* text, va_list args);
            /**
             * @brief Procedure that print given buffer char-by-char. Useful for not null terminated strings
             * @param[in] buffer Buffer to print
             */
            static void PrintBuffer(struct _LineIO* io, gsl::span<const std::uint8_t> buffer);

            /**
             * @brief Procedure that reads single line of text
             * @param io @see LineIO structure
             * @param buffer Buffer that will hold upcoming data
             * @param bufferLength Maximum length of buffer
             * @param promptChar Character to send as prompt before waiting for line read. Pass '\0' to not send any prompt.
             * @return Count of read characters
             */
            static size_t Readline(struct _LineIO* io, char* buffer, std::size_t bufferLength, char promptChar);
            /**
             * @brief Reads arbitrary number of bytes
             * @param io @see LineIO structure
             * @param outputBuffer Buffer that will be sent before reading inputBuffer
             * @param inputBuffer Buffer that will be filled
             */
            static void ExchangeBuffers(LineIO* io, gsl::span<const std::uint8_t> outputBuffer, gsl::span<uint8_t> inputBuffer);

            /**
             * @brief DMA callback
             * @param channel Channel that finished its transfer
             * @param sequenceNo Unused
             * @param userParam Pointer to uart driver object
             * @return Unused
             */
            static bool OnDma(unsigned int channel, unsigned int sequenceNo, void* userParam);

            /** @brief Line IO */
            LineIO _lineIO;
            /** @brief Synchronization group */
            EventGroup _event;
            /** @brief Line receive buffer begin */
            char* _bufferBegin;
            /** @brief Line receive buffer */
            char* _buffer;
            /** @brief Line receive buffer end */
            char* _bufferEnd;

            /** @brief Line editing flag */
            bool _lineEditing;

            /** @brief RX DMA channel */
            unsigned int _rxChannel;
            /** @brief TX DMA channel */
            unsigned int _txChannel;

            /** @brief Synchronization flags */
            struct Event
            {
                /** @brief Line end received */
                static constexpr OSEventBits LineEndReceived = 1 << 0;
                /** @brief RX transfer finished */
                static constexpr OSEventBits TransferRXFinished = 1 << 1;
                /** @brief TX transfer finished */
                static constexpr OSEventBits TransferTXFinished = 1 << 2;
            };
        };

        /** @brief DMA configuration for specified UART */
        template <std::uint8_t UartId> struct DMAConfig;

        /** @brief DMA configuration for UART 0 */
        template <> struct DMAConfig<io_map::UART_0::Id>
        {
            /** @brief DMA RX Signal for UART 0 */
            static constexpr DMADRV_PeripheralSignal_t DmaRxSignal = dmadrvPeripheralSignal_UART0_RXDATAV;
            /** @brief DMA TX Signal for UART 0 */
            static constexpr DMADRV_PeripheralSignal_t DmaTxSignal = dmadrvPeripheralSignal_UART0_TXBL;
        };

        /** @brief DMA configuration for UART 1 */
        template <> struct DMAConfig<io_map::UART_1::Id>
        {
            /** @brief DMA RX Signal for UART 1 */
            static constexpr DMADRV_PeripheralSignal_t DmaRxSignal = dmadrvPeripheralSignal_UART1_RXDATAV;
            /** @brief DMA TRX Signal for UART 1 */
            static constexpr DMADRV_PeripheralSignal_t DmaTxSignal = dmadrvPeripheralSignal_UART1_TXBL;
        };

        template <typename UartPort> void UART<UartPort>::LineEditing(bool enable)
        {
            this->_lineEditing = enable;
        }

        template <typename UartPort>
        UART<UartPort>::UART()
            : _bufferBegin(nullptr), _buffer(nullptr), _bufferEnd(nullptr), _lineEditing(false), _rxChannel(0), _txChannel(0)
        {
            this->_lineIO.extra = this;
            this->_lineIO.PrintBuffer = PrintBuffer;
            this->_lineIO.Puts = Puts;
            this->_lineIO.ExchangeBuffers = ExchangeBuffers;
            this->_lineIO.Readline = Readline;
            this->_lineIO.VPrintf = VPrintf;

            this->_event.Initialize();
        }

        template <typename UartPort> void UART<UartPort>::Initialize()
        {
            USART_InitAsync_TypeDef init = USART_INITASYNC_DEFAULT;
            init.baudrate = UartPort::Baudrate;
            init.enable = usartDisable;

            CMU_ClockEnable(UartPort::Clock, true);
            USART_InitAsync(UartPort::Peripheral, &init);

            UartPort::Peripheral->ROUTE |= UART_ROUTE_TXPEN | UART_ROUTE_RXPEN | UartPort::Location;

            USART_IntEnable(UartPort::Peripheral, UART_IEN_RXDATAV);

            NVIC_SetPriority(UartPort::Interrupt, UartPort::InterruptPriority);
            NVIC_SetPriority(UartPort::WakeUpInterrupt, UartPort::WakeUpInterruptPriority);
            NVIC_EnableIRQ(UartPort::WakeUpInterrupt);

            USART_Enable(UartPort::Peripheral, usartEnable);

            DMADRV_AllocateChannel(&this->_rxChannel, nullptr);
            DMADRV_AllocateChannel(&this->_txChannel, nullptr);
        }

        template <typename UartPort> LineIO& UART<UartPort>::GetLineIO()
        {
            return this->_lineIO;
        }

        template <typename UartPort> void UART<UartPort>::Puts(struct _LineIO* io, const char* s)
        {
            UNREFERENCED_PARAMETER(io);

            while (*s != '\0')
            {
                USART_Tx(UartPort::Peripheral, *s);
                s++;
            }
        }

        template <typename UartPort> void UART<UartPort>::VPrintf(struct _LineIO* io, const char* text, va_list args)
        {
            char buf[255] = {0};

            vsniprintf(buf, sizeof(buf), text, args);

            UART::Puts(io, buf);
        }

        template <typename UartPort> void UART<UartPort>::WriteBuffer(gsl::span<const std::uint8_t> buffer)
        {
            USART_IntDisable(UartPort::Peripheral, UART_IEN_RXDATAV);

            _event.Clear(Event::TransferTXFinished);

            DMADRV_MemoryPeripheral(_txChannel,
                DMAConfig<UartPort::Id>::DmaTxSignal,
                reinterpret_cast<void*>(const_cast<std::uint32_t*>(&UartPort::Peripheral->TXDATA)),
                const_cast<std::uint8_t*>(buffer.data()),
                true,
                buffer.size(),
                dmadrvDataSize1,
                OnDma,
                this);

            _event.WaitAll(Event::TransferTXFinished, true, InfiniteTimeout);

            USART_IntEnable(UartPort::Peripheral, UART_IEN_RXDATAV);
        }

        template <typename UartPort> void UART<UartPort>::PrintBuffer(struct _LineIO* io, gsl::span<const std::uint8_t> buffer)
        {
            auto This = reinterpret_cast<UART*>(io->extra);
            This->WriteBuffer(buffer);
        }

        template <typename UartPort> void UART<UartPort>::ReadBuffer(gsl::span<const std::uint8_t> buffer)
        {
            USART_IntDisable(UartPort::Peripheral, UART_IEN_RXDATAV);

            _event.Clear(Event::TransferRXFinished);

            DMADRV_PeripheralMemory(_rxChannel,
                DMAConfig<UartPort::Id>::DmaRxSignal,
                buffer.data(),
                reinterpret_cast<void*>(const_cast<std::uint32_t*>(&UartPort::Peripheral->RXDATA)),
                true,
                buffer.size(),
                dmadrvDataSize1,
                OnDma,
                this);

            _event.WaitAll(Event::TransferRXFinished, true, InfiniteTimeout);

            USART_IntEnable(UartPort::Peripheral, UART_IEN_RXDATAV);
        }

        template <typename UartPort>
        size_t UART<UartPort>::Readline(struct _LineIO* io, char* buffer, std::size_t bufferLength, char promptChar)
        {
            auto This = reinterpret_cast<UART*>(io->extra);

            This->_event.Clear(Event::LineEndReceived);

            This->_bufferBegin = buffer;
            This->_buffer = buffer;
            This->_bufferEnd = buffer + bufferLength;

            NVIC_EnableIRQ(UartPort::Interrupt);

            if (promptChar != '\0')
            {
                USART_Tx(UartPort::Peripheral, promptChar);
            }

            This->_event.WaitAny(Event::LineEndReceived, true, InfiniteTimeout);

            NVIC_DisableIRQ(UartPort::Interrupt);

            *(This->_buffer - 1) = 0;

            return (This->_buffer - buffer);
        }

        template <typename UartPort>
        void UART<UartPort>::ExchangeBuffers(LineIO* io, gsl::span<const std::uint8_t> outputBuffer, gsl::span<uint8_t> inputBuffer)
        {
            auto This = reinterpret_cast<UART*>(io->extra);

            USART_IntDisable(UartPort::Peripheral, UART_IEN_RXDATAV);

            This->_event.Clear(Event::TransferTXFinished | Event::TransferRXFinished);

            DMADRV_PeripheralMemory(This->_rxChannel,
                DMAConfig<UartPort::Id>::DmaRxSignal,
                inputBuffer.data(),
                reinterpret_cast<void*>(const_cast<std::uint32_t*>(&UartPort::Peripheral->RXDATA)),
                true,
                inputBuffer.size(),
                dmadrvDataSize1,
                OnDma,
                This);

            DMADRV_MemoryPeripheral(This->_txChannel,
                DMAConfig<UartPort::Id>::DmaTxSignal,
                reinterpret_cast<void*>(const_cast<std::uint32_t*>(&UartPort::Peripheral->TXDATA)),
                const_cast<std::uint8_t*>(outputBuffer.data()),
                true,
                outputBuffer.size(),
                dmadrvDataSize1,
                OnDma,
                This);

            This->_event.WaitAll(Event::TransferTXFinished | Event::TransferRXFinished, true, InfiniteTimeout);

            USART_IntEnable(UartPort::Peripheral, UART_IEN_RXDATAV);
        }

        template <typename UartPort> __attribute__((optimize("O3"))) void UART<UartPort>::OnReceived()
        {
            auto b = UartPort::Peripheral->RXDATA & 0x0FF;
            *this->_buffer = b;
            this->_buffer++;
            if (b == '\n' || this->_buffer == this->_bufferEnd)
            {
                NVIC_SetPendingIRQ(UartPort::WakeUpInterrupt);
            }

            if (this->_lineEditing)
            {
                switch (b)
                {
                    case '\b':
                        if (this->_buffer - this->_bufferBegin > 2)
                        {
                            this->_buffer -= 2;
                        }
                        else
                        {
                            --this->_buffer;
                        }
                        break;
                }
            }
        }

        template <typename UartPort> void UART<UartPort>::OnWakeUpInterrupt()
        {
            this->_event.SetISR(Event::LineEndReceived);
        }

        template <typename UartPort> bool UART<UartPort>::OnDma(unsigned int channel, unsigned int /*sequenceNo*/, void* userParam)
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

#endif /* LIBS_DRIVERS_UART_INCLUDE_UART_UART_H_ */
