#ifndef LIBS_DRIVERS_LEUART_INCLUDE_LEUART_LINE_IO_H_
#define LIBS_DRIVERS_LEUART_INCLUDE_LEUART_LINE_IO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>

typedef struct
{
    void* extra;
    void (*Puts)(const char* s);
    void (*VPrintf)(const char* text, va_list args);
    size_t (*Readline)(char* buffer, size_t bufferLength)
} LineIO;

#ifdef __cplusplus
}
#endif

#endif /* LIBS_DRIVERS_LEUART_INCLUDE_LEUART_LINE_IO_H_ */
