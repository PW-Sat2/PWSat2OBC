#ifndef LIBS_DRIVERS_LEUART_INCLUDE_LEUART_LINE_IO_H_
#define LIBS_DRIVERS_LEUART_INCLUDE_LEUART_LINE_IO_H_

#include <stdarg.h>
#include <stddef.h>
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
    void (*PrintBuffer)(gsl::span<const std::uint8_t> buffer);

    /**
     * @brief Procedure that reads single line of text
     * @param io @see LineIO structure
     * @param buffer Buffer that will hold upcoming data
     * @param bufferLength Maximum length of buffer
     * @return Count of read characters
     */
    size_t (*Readline)(struct _LineIO* io, char* buffer, size_t bufferLength);

    /**
     * @brief Reads arbitrary number of bytes
     * @param io @see LineIO structure
     * @param outputBuffer Buffer that will be sent before reading inputBuffer
     * @param inputBuffer Buffer that will be filled
     */
    void (*ExchangeBuffers)(struct _LineIO* io, gsl::span<const std::uint8_t> outputBuffer, gsl::span<std::uint8_t> inputBuffer);
} LineIO;

#endif /* LIBS_DRIVERS_LEUART_INCLUDE_LEUART_LINE_IO_H_ */
