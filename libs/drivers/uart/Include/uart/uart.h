#ifndef LIBS_DRIVERS_UART_INCLUDE_UART_UART_H_
#define LIBS_DRIVERS_UART_INCLUDE_UART_UART_H_

#include <stdarg.h>
#include <cstdint>
#include <gsl/span>
#include "base/os.h"
#include "leuart/line_io.h"

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
        class UART
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
            static void PrintBuffer(gsl::span<const std::uint8_t> buffer);
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

        void UART::LineEditing(bool enable)
        {
            this->_lineEditing = enable;
        }
    }
}

#endif /* LIBS_DRIVERS_UART_INCLUDE_UART_UART_H_ */
