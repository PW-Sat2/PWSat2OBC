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

          private:
            static void Puts(struct _LineIO* io, const char* s);
            static void VPrintf(struct _LineIO* io, const char* text, va_list args);
            static void PrintBuffer(gsl::span<const std::uint8_t> buffer);
            static size_t Readline(struct _LineIO* io, char* buffer, std::size_t bufferLength);
            static void ExchangeBuffers(LineIO* io, gsl::span<const std::uint8_t> outputBuffer, gsl::span<uint8_t> inputBuffer);

            LineIO _lineIO;
            Queue<std::uint8_t, 50> _queue;
        };
    }
}

#endif /* LIBS_DRIVERS_UART_INCLUDE_UART_UART_H_ */
