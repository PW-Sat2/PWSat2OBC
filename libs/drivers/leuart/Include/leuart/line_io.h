#ifndef LIBS_DRIVERS_LEUART_INCLUDE_LEUART_LINE_IO_H_
#define LIBS_DRIVERS_LEUART_INCLUDE_LEUART_LINE_IO_H_

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

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
     *
     * @param io Procedure that reads single line of text
     * @param buffer Buffer that will hold upcoming data
     * @param bufferLength Maximum length of buffer
     * @return Count of read characters
     */
    size_t (*Readline)(struct _LineIO* io, char* buffer, size_t bufferLength);
} LineIO;

#ifdef __cplusplus
}
#endif

#endif /* LIBS_DRIVERS_LEUART_INCLUDE_LEUART_LINE_IO_H_ */
