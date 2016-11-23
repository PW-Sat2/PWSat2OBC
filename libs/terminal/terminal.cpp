#include <string.h>

#include "base/os.h"
#include "logger/logger.h"
#include "system.h"
#include "terminal.h"

static void parseCommandLine(char line[],
    char** commandName,
    char** arguments,
    uint16_t* argc,
    uint8_t maxArgsCount //
    )
{
    *argc = 0;

    char* ptr;
    char* token = strtok_r(line, " ", &ptr);

    *commandName = token;

    token = strtok_r(NULL, " ", &ptr);

    while (token != NULL && *argc < maxArgsCount)
    {
        arguments[*argc] = token;
        (*argc)++;

        token = strtok_r(NULL, " ", &ptr);
    }
}

void TerminalPuts(Terminal* terminal, const char* text)
{
    terminal->stdio->Puts(terminal->stdio, text);
}

void TerminalSendNewLine(Terminal* terminal)
{
    TerminalPuts(terminal, "\n");
}

static void terminalSendPrefix(Terminal* terminal)
{
    TerminalPuts(terminal, ">");
}

void TerminalPrintf(Terminal* terminal, const char* text, ...)
{
    va_list args;
    va_start(args, text);

    terminal->stdio->VPrintf(terminal->stdio, text, args);

    va_end(args);
}

static void terminalHandleCommand(Terminal* terminal, char* buffer)
{
    char* commandName;
    uint16_t argc = 0;
    char* args[8] = {0};

    parseCommandLine(buffer, &commandName, args, &argc, COUNT_OF(args));
    const TerminalCommandDescription* commands = terminal->commandList;
    const uint32_t commandCount = terminal->commandCount;
    for (size_t i = 0; i < commandCount; i++)
    {
        if (strcmp(commandName, commands[i].name) == 0)
        {
            commands[i].handler(argc, args);
            TerminalSendNewLine(terminal);
        }
    }
}

static void handleIncomingChar(Terminal& terminal)
{
    bool firstRun = true;

    while (1)
    {
        char input_buffer[100] = {0};

        if (!firstRun)
        {
            terminalSendPrefix(&terminal);
        }

        firstRun = false;

        terminal.stdio->Readline(terminal.stdio, input_buffer, COUNT_OF(input_buffer));

        LOGF(LOG_LEVEL_INFO, "Received line %s", input_buffer);

        terminalHandleCommand(&terminal, input_buffer);
    }
}

void TerminalInit(Terminal* terminal, LineIO* stdio)
{
    terminal->stdio = stdio;

    if (OS_RESULT_FAILED(System::CreateTask(handleIncomingChar, *terminal, "terminalIn", 2500, 4)))
    {
        LOG(LOG_LEVEL_ERROR, "Error. Cannot create terminalQueue.");
    }
    else
    {
        stdio->Puts(stdio, "@");
    }
}

void TerminalSetCommandList(Terminal* terminal, const TerminalCommandDescription* commandList, uint32_t commandCount)
{
    terminal->commandList = commandList;
    terminal->commandCount = commandCount;
}
