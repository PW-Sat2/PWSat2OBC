#ifndef LIBS_DRIVERS_LEUART_INCLUDE_LEUART_LINE_IO_H_
#define LIBS_DRIVERS_LEUART_INCLUDE_LEUART_LINE_IO_H_

#include <stdarg.h>
#include <stddef.h>
#include <chrono>
#include <cstdint>
#include <gsl/span>

/**
 * @brief API for line based interface
 */
typedef struct _LineIO
{
    /**
     * @brief Extra value for use by underlying implementation
     */
    void* extra;
    /**
     * @brief Procedure that outputs given string
     * @param io @see LineIO structure
     * @param s Text to output
     */
    void (*Puts)(struct _LineIO* io, const char* s);
    /**
     * @brief Procedure that outputs formatted string
     * @param io @see LineIO structure
     * @param text Format string
     * @param args Format atrguments as @see a_list
     */
    void (*VPrintf)(struct _LineIO* io, const char* text, va_list args);

    /**
     * @brief Procedure that print given buffer char-by-char. Useful for not null terminated strings
     * @param[in] buffer Buffer to print
     */
    void (*PrintBuffer)(struct _LineIO* io, gsl::span<const std::uint8_t> buffer);

    /**
     * @brief Procedure that reads single line of text
     * @param io @see LineIO structure
     * @param buffer Buffer that will hold upcoming data
     * @param bufferLength Maximum length of buffer
     * @param promptChar Character to send as prompt before waiting for line read. Pass '\0' to not send any prompt.
     * @return Count of read characters
     */
    size_t (*Readline)(struct _LineIO* io, char* buffer, size_t bufferLength, char promptChar);

    /**
     * @brief Procedure that reads specified buffer
     * @param io @see LineIO structure
     * @param buffer Buffer that will hold upcoming data
     * @param timeout Timeout for read command
     * @return True if read succeeded, false otherwise
     */
    bool (*Read)(struct _LineIO* io, gsl::span<std::uint8_t> buffer, std::chrono::milliseconds timeout);

    /**
     * @brief Reads arbitrary number of bytes
     * @param io @see LineIO structure
     * @param outputBuffer Buffer that will be sent before reading inputBuffer
     * @param inputBuffer Buffer that will be filled
     * @param timeout Timeout for read operation
     * @return False if timeout occured, true otherwise
     */
    bool (*ExchangeBuffers)(struct _LineIO* io,
        gsl::span<const std::uint8_t> outputBuffer,
        gsl::span<std::uint8_t> inputBuffer,
        std::chrono::milliseconds timeout);
} LineIO;

#endif /* LIBS_DRIVERS_LEUART_INCLUDE_LEUART_LINE_IO_H_ */
