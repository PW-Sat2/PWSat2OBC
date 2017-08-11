#ifndef LIBS_DRIVERS_UART_INCLUDE_UART_UART_H_
#define LIBS_DRIVERS_UART_INCLUDE_UART_UART_H_

#include <stdarg.h>
#include <stdio.h>
#include <chrono>
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
        using namespace std::chrono_literals;

        /**
         * @defgroup uart_driver UART Driver
         * @ingroup peripheral_drivers
         */

        /**
         * @brief UART driver
         * @ingroup uart_driver
         */
        template <typename UartPort> class UART final : public ILineIO
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
             * @brief Reads date from UART port into buffer using DMA transfer
             * @param buffer Buffer that will hold upcoming data
             * @param timeout Timeout for read operation
             * @return False if timeout occured, true otherwise
             */
            virtual bool Read(gsl::span<std::uint8_t> buffer, std::chrono::milliseconds timeout) override;

            /**
             * @brief Procedure that outputs given string
             * @param s Text to output
             */
            virtual void Puts(const char* s) override;

            /**
             * @brief Procedure that outputs formatted string
             * @param text Format string
             * @param args Format atrguments as @see a_list
             */
            virtual void VPrintf(const char* text, va_list args) override;

            /**
             * @brief Procedure that print given buffer char-by-char. Useful for not null terminated strings
             * @param[in] buffer Buffer to print
             */
            virtual void PrintBuffer(gsl::span<const std::uint8_t> buffer) override;

            /**
             * @brief Procedure that reads single line of text
             * @param buffer Buffer that will hold upcoming data
             * @param bufferLength Maximum length of buffer
             * @param promptChar Character to send as prompt before waiting for line read. Pass '\0' to not send any prompt.
             * @return Count of read characters
             */
            virtual size_t Readline(char* buffer, std::size_t bufferLength, char promptChar) override;

            /**
             * @brief Reads arbitrary number of bytes
             * @param outputBuffer Buffer that will be sent before reading inputBuffer
             * @param inputBuffer Buffer that will be filled
             * @param timeout Timeout for read operation
             * @return False if timeout occurred, true otherwise
             */
            virtual bool ExchangeBuffers(                   //
                gsl::span<const std::uint8_t> outputBuffer, //
                gsl::span<uint8_t> inputBuffer,             //
                std::chrono::milliseconds timeout) override;

          private:
            /**
             * @brief DMA callback
             * @param channel Channel that finished its transfer
             * @param sequenceNo Unused
             * @param userParam Pointer to uart driver object
             * @return Unused
             */
            static bool OnDma(unsigned int channel, unsigned int sequenceNo, void* userParam);

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
            this->_event.Initialize();
        }

        template <typename UartPort> void UART<UartPort>::Initialize()
        {
            USART_InitAsync_TypeDef init = USART_INITASYNC_DEFAULT;
            init.baudrate = UartPort::Baudrate;
            init.enable = usartDisable;

            CMU_ClockEnable(UartPort::Clock, true);
            USART_InitAsync(UartPort::Peripheral, &init);

            UartPort::Peripheral->ROUTE |= USART_ROUTE_CLKPEN | USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | UartPort::Location;

            USART_IntEnable(UartPort::Peripheral, UART_IEN_RXDATAV);

            NVIC_SetPriority(UartPort::Interrupt, UartPort::InterruptPriority);
            NVIC_SetPriority(UartPort::WakeUpInterrupt, UartPort::WakeUpInterruptPriority);
            NVIC_EnableIRQ(UartPort::WakeUpInterrupt);

            USART_Enable(UartPort::Peripheral, usartEnable);

            DMADRV_AllocateChannel(&this->_rxChannel, nullptr);
            DMADRV_AllocateChannel(&this->_txChannel, nullptr);
        }

        template <typename UartPort> void UART<UartPort>::Puts(const char* s)
        {
            while (*s != '\0')
            {
                USART_Tx(UartPort::Peripheral, *s);
                s++;
            }
        }

        template <typename UartPort> void UART<UartPort>::VPrintf(const char* text, va_list args)
        {
            char buf[255] = {0};

            vsniprintf(buf, sizeof(buf), text, args);

            Puts(buf);
        }

        template <typename UartPort> void UART<UartPort>::PrintBuffer(gsl::span<const std::uint8_t> buffer)
        {
            USART_IntDisable(UartPort::Peripheral, UART_IEN_RXDATAV);

            for (decltype(buffer.size()) offset = 0; offset < buffer.size(); offset += 1024)
            {
                auto part = buffer.subspan(offset, std::min(1024, buffer.size() - offset));

                _event.Clear(Event::TransferTXFinished);

                DMADRV_MemoryPeripheral(_txChannel,
                    DMAConfig<UartPort::Id>::DmaTxSignal,
                    reinterpret_cast<void*>(const_cast<std::uint32_t*>(&UartPort::Peripheral->TXDATA)),
                    const_cast<std::uint8_t*>(part.data()),
                    true,
                    part.size(),
                    dmadrvDataSize1,
                    OnDma,
                    this);

                _event.WaitAll(Event::TransferTXFinished, true, 30s);
            }

            USART_IntEnable(UartPort::Peripheral, UART_IEN_RXDATAV);
        }

        template <typename UartPort> bool UART<UartPort>::Read(gsl::span<std::uint8_t> buffer, std::chrono::milliseconds timeout)
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

            auto result = (_event.WaitAll(Event::TransferRXFinished, true, timeout) & Event::TransferRXFinished) != 0;

            USART_IntEnable(UartPort::Peripheral, UART_IEN_RXDATAV);

            return result;
        }

        template <typename UartPort> size_t UART<UartPort>::Readline(char* buffer, std::size_t bufferLength, char promptChar)
        {
            _event.Clear(Event::LineEndReceived);

            _bufferBegin = buffer;
            _buffer = buffer;
            _bufferEnd = buffer + bufferLength;

            NVIC_EnableIRQ(UartPort::Interrupt);

            if (promptChar != '\0')
            {
                USART_Tx(UartPort::Peripheral, promptChar);
            }

            _event.WaitAny(Event::LineEndReceived, true, InfiniteTimeout);

            NVIC_DisableIRQ(UartPort::Interrupt);

            *(_buffer - 1) = 0;

            return (_buffer - buffer);
        }

        template <typename UartPort>
        bool UART<UartPort>::ExchangeBuffers(gsl::span<const std::uint8_t> outputBuffer, //
            gsl::span<uint8_t> inputBuffer,                                              //
            std::chrono::milliseconds timeout)                                           //
        {
            USART_IntDisable(UartPort::Peripheral, UART_IEN_RXDATAV);

            _event.Clear(Event::TransferTXFinished | Event::TransferRXFinished);

            DMADRV_PeripheralMemory(_rxChannel,
                DMAConfig<UartPort::Id>::DmaRxSignal,
                inputBuffer.data(),
                reinterpret_cast<void*>(const_cast<std::uint32_t*>(&UartPort::Peripheral->RXDATA)),
                true,
                inputBuffer.size(),
                dmadrvDataSize1,
                OnDma,
                this);

            DMADRV_MemoryPeripheral(_txChannel,
                DMAConfig<UartPort::Id>::DmaTxSignal,
                reinterpret_cast<void*>(const_cast<std::uint32_t*>(&UartPort::Peripheral->TXDATA)),
                const_cast<std::uint8_t*>(outputBuffer.data()),
                true,
                outputBuffer.size(),
                dmadrvDataSize1,
                OnDma,
                this);

            auto result =
                (_event.WaitAll(Event::TransferTXFinished | Event::TransferRXFinished, true, timeout) & Event::TransferRXFinished) != 0;

            USART_IntEnable(UartPort::Peripheral, UART_IEN_RXDATAV);

            return result;
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
