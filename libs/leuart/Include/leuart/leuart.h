#ifndef SRC_LEUART_H_
#define SRC_LEUART_H_

#include <stdarg.h>

void leuartInit(xQueueHandle sink);
void leuartPuts(const char* buffer);
void leuartPrintf(const char * text, ...);
void leuartvPrintf(const char * text, va_list args);
void leuartPutc(const char c);

#endif
