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

void Terminal::Puts(const char* text)
{
    this->stdio.Puts(&this->stdio, text);
}

void Terminal::NewLine()
{
    this->Puts("\n");
}

void Terminal::SendPrefix()
{
    this->Puts(">");
}

void Terminal::Printf(const char* text, ...)
{
    va_list args;
    va_start(args, text);

    this->stdio.VPrintf(&this->stdio, text, args);

    va_end(args);
}

void Terminal::PrintBuffer(gsl::span<const char> buffer)
{
    this->stdio.PrintBuffer(buffer);
}

void Terminal::HandleCommand(char* buffer)
{
    char* commandName;
    uint16_t argc = 0;
    char* args[8] = {0};

    parseCommandLine(buffer, &commandName, args, &argc, COUNT_OF(args));

    for (auto& command : this->commandList)
    {
        if (strcmp(commandName, command.name) == 0)
        {
            command.handler(argc, args);

            this->NewLine();

            return;
        }
    }
}

void Terminal::Loop(Terminal* terminal)
{
    bool firstRun = true;

    while (1)
    {
        char input_buffer[100] = {0};

        if (!firstRun)
        {
            terminal->SendPrefix();
        }

        firstRun = false;

        terminal->stdio.Readline(&terminal->stdio, input_buffer, COUNT_OF(input_buffer));

        LOGF(LOG_LEVEL_INFO, "Received line %s", input_buffer);

        terminal->HandleCommand(input_buffer);
    }
}

Terminal::Terminal(LineIO& stdio)
    : stdio(stdio), //
      task("Terminal", this, Terminal::Loop)
{
}

void Terminal::Initialize()
{
    if (OS_RESULT_FAILED(this->task.Create()))
    {
        LOG(LOG_LEVEL_ERROR, "Error. Cannot create terminalQueue.");
    }
    else
    {
        this->stdio.Puts(&stdio, "@");
    }
}

void Terminal::SetCommandList(gsl::span<const TerminalCommandDescription> commands)
{
    this->commandList = commands;
}
