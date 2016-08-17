#ifndef SRC_LEUART_H_
#define SRC_LEUART_H_

#include <stdarg.h>
#include "line_io.h"

void LeuartLineIOInit(LineIO* io);

void leuartInit(xQueueHandle sink);
void leuartPuts(const char* buffer);
void leuartPrintf(const char* text, ...);
void leuartvPrintf(const char* text, va_list args);

#endif
