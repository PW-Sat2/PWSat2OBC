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
struct ILineIO
{
    /**
     * @brief Procedure that outputs given string
     * @param s Text to output
     */
    virtual void Puts(const char* s) = 0;

    /**
     * @brief Procedure that outputs formatted string
     * @param text Format string
     * @param args Format atrguments as @see a_list
     */
    virtual void VPrintf(const char* text, va_list args) = 0;

    /**
     * @brief Procedure that print given buffer char-by-char. Useful for not null terminated strings
     * @param[in] buffer Buffer to print
     */
    virtual void PrintBuffer(gsl::span<const std::uint8_t> buffer) = 0;

    /**
     * @brief Procedure that reads single line of text
     * @param buffer Buffer that will hold upcoming data
     * @param bufferLength Maximum length of buffer
     * @param promptChar Character to send as prompt before waiting for line read. Pass '\0' to not send any prompt.
     * @return Count of read characters
     */
    virtual size_t Readline(char* buffer, size_t bufferLength, char promptChar) = 0;

    /**
     * @brief Procedure that reads specified buffer
     * @param buffer Buffer that will hold upcoming data
     * @param timeout Timeout for read command
     * @return True if read succeeded, false otherwise
     */
    virtual bool Read(gsl::span<std::uint8_t> buffer, std::chrono::milliseconds timeout) = 0;

    /**
     * @brief Reads arbitrary number of bytes
     * @param outputBuffer Buffer that will be sent before reading inputBuffer
     * @param inputBuffer Buffer that will be filled
     * @param timeout Timeout for read operation
     * @return False if timeout occurred, true otherwise
     */
    virtual bool ExchangeBuffers(                   //
        gsl::span<const std::uint8_t> outputBuffer, //
        gsl::span<std::uint8_t> inputBuffer,        //
        std::chrono::milliseconds timeout) = 0;     //
};

#endif /* LIBS_DRIVERS_LEUART_INCLUDE_LEUART_LINE_IO_H_ */
