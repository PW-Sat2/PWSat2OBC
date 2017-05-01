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
        class UART
        {
          public:
            UART();
            void Initialize();

            LineIO& GetLineIO();

            void OnReceived();
            void OnWakeUpInterrupt();

          private:
            static void Puts(struct _LineIO* io, const char* s);
            static void VPrintf(struct _LineIO* io, const char* text, va_list args);
            static void PrintBuffer(gsl::span<const std::uint8_t> buffer);
            static size_t Readline(struct _LineIO* io, char* buffer, std::size_t bufferLength);
            static void ExchangeBuffers(LineIO* io, gsl::span<const std::uint8_t> outputBuffer, gsl::span<uint8_t> inputBuffer);

            static bool OnDma(unsigned int channel, unsigned int sequenceNo, void* userParam);

            LineIO _lineIO;
            Queue<std::uint8_t, 50> _queue;
            EventGroup _event;
            char* _buffer;

            unsigned int _rxChannel;
            unsigned int _txChannel;

            struct Event
            {
                static constexpr OSEventBits LineEndReceived = 1 << 0;
                static constexpr OSEventBits TransferRXFinished = 1 << 1;
                static constexpr OSEventBits TransferTXFinished = 1 << 2;
            };
        };
    }
}

#endif /* LIBS_DRIVERS_UART_INCLUDE_UART_UART_H_ */
