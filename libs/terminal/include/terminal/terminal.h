#ifndef SRC_TERMINAL_H_
#define SRC_TERMINAL_H_

#include <stdint.h>
#include "leuart/line_io.h"

typedef void (*TerminalCommandHandler)(uint16_t argc, char* argv[]);

typedef struct
{
    const char* name;
    TerminalCommandHandler handler;
} TerminalCommandDescription;

typedef struct
{
    LineIO* stdio;
    const TerminalCommandDescription* commandList;
    uint32_t commandCount;
} Terminal;

void TerminalInit(Terminal* terminal, LineIO* stdio);
void TerminalSetCommandList(Terminal* terminal, const TerminalCommandDescription* commandList, uint32_t commandCount);

void TerminalSendNewLine(Terminal* terminal);
void TerminalPrintf(Terminal* terminal, const char* text, ...);
void TerminalPuts(Terminal* terminal, const char* text);

#endif
