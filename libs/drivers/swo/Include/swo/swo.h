#ifndef SRC_SWO_H_
#define SRC_SWO_H_

#include <stdarg.h>
#include <stdint.h>

void SwoEnable(void);

void SwoPutsOnChannel(uint8_t channel, const char* str);

void SwoPrintfOnChannel(uint8_t channel, const char* format, ...);

void SwoVPrintfOnChannel(uint8_t channel, const char* format, va_list arguments);

static inline void SwoPuts(const char* str)
{
    SwoPutsOnChannel(0, str);
}

static inline void SwoVPrintf(const char* format, va_list arguments)
{
    SwoVPrintfOnChannel(0, format, arguments);
}

#endif /* SRC_SWO_H_ */
