#ifndef SRC_TERMINAL_H_
#define SRC_TERMINAL_H_

#include "leuart/line_io.h"

void TerminalInit(void);
void TerminalSendNewLine(void);
void TerminalPrintf(const char* text, ...);
void TerminalPuts(const char* text);

#endif
