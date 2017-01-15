#include <string.h>
#include <array>

#include "base/os.h"
#include "base/reader.h"
#include "base/writer.h"
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

Terminal::Terminal(LineIO& stdio)
    : _stdio(stdio), //
      _task("Terminal", this, Terminal::Loop)
{
}

void Terminal::Puts(const char* text)
{
    this->_stdio.Puts(&this->_stdio, text);
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

    this->_stdio.VPrintf(&this->_stdio, text, args);

    va_end(args);
}

void Terminal::PrintBuffer(gsl::span<const std::uint8_t> buffer)
{
    this->_stdio.PrintBuffer(buffer);
}

void Terminal::ReadBuffer(gsl::span<std::uint8_t> buffer)
{
    this->_stdio.ReadBuffer(&this->_stdio, buffer);
}

void Terminal::HandleCommand(char* buffer)
{
    char* commandName;
    uint16_t argc = 0;
    char* args[8] = {0};

    parseCommandLine(buffer, &commandName, args, &argc, COUNT_OF(args));

    for (auto& command : this->_commandList)
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

        terminal->_stdio.Readline(&terminal->_stdio, input_buffer, COUNT_OF(input_buffer));

        terminal->HandleCommand(input_buffer);
    }
}

void Terminal::Initialize()
{
    if (OS_RESULT_FAILED(this->_task.Create()))
    {
        LOG(LOG_LEVEL_ERROR, "Error. Cannot create terminalQueue.");
    }
    else
    {
        this->_stdio.Puts(&_stdio, "@");
    }
}

void Terminal::SetCommandList(gsl::span<const TerminalCommandDescription> commands)
{
    this->_commandList = commands;
}

TerminalPartialRetrival::TerminalPartialRetrival(Terminal& terminal, gsl::span<uint8_t> buffer)
    : _terminal(terminal), _buffer(buffer), _remainingLength(0)
{
}

void TerminalPartialRetrival::Start()
{
    this->_terminal.Puts("#");

    std::array<uint8_t, sizeof(std::uint32_t)> lengthBuffer;

    this->_terminal.ReadBuffer(lengthBuffer);

    Reader r(lengthBuffer);
    this->_remainingLength = r.ReadDoubleWordLE();
}

Option<gsl::span<uint8_t>> TerminalPartialRetrival::ReadPart()
{
    if (this->_remainingLength == 0)
    {
        return Option<gsl::span<uint8_t>>::None();
    }

    auto partLength = std::min<std::size_t>(this->_buffer.size(), this->_remainingLength);
    auto part = this->_buffer.subspan(0, partLength);

    std::array<uint8_t, sizeof(std::uint32_t)> partLengthBuffer;
    Writer w(partLengthBuffer);
    w.WriteDoubleWordLE(static_cast<std::uint32_t>(part.size()));

    this->_terminal.PrintBuffer(partLengthBuffer);

    this->_terminal.ReadBuffer(part);

    this->_remainingLength -= part.size();

    return Option<gsl::span<uint8_t>>::Some(part);
}
