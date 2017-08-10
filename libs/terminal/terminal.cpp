#include <string.h>
#include <array>

#include "base/os.h"
#include "base/reader.h"
#include "base/writer.h"
#include "logger/logger.h"
#include "system.h"
#include "terminal.h"

using namespace std::literals;

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

Terminal::Terminal(ILineIO& stdio)
    : _stdio(stdio), //
      _task("Terminal", this, Terminal::Loop)
{
}

void Terminal::Puts(const char* text)
{
    this->_stdio.Puts(text);
}

void Terminal::NewLine()
{
    this->Puts("\n");
}

void Terminal::Printf(const char* text, ...)
{
    va_list args;
    va_start(args, text);

    this->_stdio.VPrintf(text, args);

    va_end(args);
}

void Terminal::PrintBuffer(gsl::span<const std::uint8_t> buffer)
{
    this->_stdio.PrintBuffer(buffer);
}

void Terminal::ExchangeBuffers(gsl::span<const std::uint8_t> outputBuffer, gsl::span<std::uint8_t> inputBuffer)
{
    this->_stdio.ExchangeBuffers(outputBuffer, inputBuffer, 5s);
}

void Terminal::HandleCommand(char* buffer)
{
    char* commandName = nullptr;
    uint16_t argc = 0;
    char* args[8] = {0};

    parseCommandLine(buffer, &commandName, args, &argc, COUNT_OF(args));

    if (commandName == nullptr || strlen(commandName) == 0)
    {
        this->NewLine();
        return;
    }

    for (auto& command : this->_commandList)
    {
        if (strcmp(commandName, command.name) == 0)
        {
            command.handler(argc, args);

            this->NewLine();

            return;
        }
    }

    LOGF(LOG_LEVEL_WARNING, "Unknown command '%s'", commandName);
}

void Terminal::Loop(Terminal* terminal)
{
    char prompt = '\0';

    while (1)
    {
        char input_buffer[100] = {0};

        terminal->_stdio.Readline(input_buffer, COUNT_OF(input_buffer), prompt);

        terminal->HandleCommand(input_buffer);

        prompt = '>';
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
        this->_stdio.Puts("@");
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
    std::array<uint8_t, sizeof(std::uint32_t)> lengthBuffer;

    uint8_t prompt = static_cast<uint8_t>('#');

    this->_terminal.ExchangeBuffers(gsl::make_span(&prompt, 1), lengthBuffer);

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

    this->_terminal.ExchangeBuffers(partLengthBuffer, part);

    this->_remainingLength -= part.size();

    return Option<gsl::span<uint8_t>>::Some(part);
}
