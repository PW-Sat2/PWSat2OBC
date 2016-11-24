#ifndef SRC_TERMINAL_H_
#define SRC_TERMINAL_H_

#include <cstdint>
#include <gsl/span>
#include "base/os.h"
#include "leuart/line_io.h"
#include "utils.h"

using TerminalCommandHandler = void (*)(std::uint16_t argc, char* argv[]);

struct TerminalCommandDescription
{
    const char* name;
    TerminalCommandHandler handler;
};

class Terminal
{
  public:
    Terminal(LineIO& stdio);
    void SetCommandList(gsl::span<const TerminalCommandDescription> commands);
    void Initialize();
    void NewLine();
    void Printf(const char* text, ...);
    void Puts(const char* text);
    void PrintBuffer(gsl::span<const char> buffer);

  private:
    void SendPrefix();
    void HandleCommand(char* buffer);
    static void Loop(Terminal*);

    LineIO& stdio;
    Task<Terminal*, 2_KB, TaskPriority::P4> task;

    gsl::span<const TerminalCommandDescription> commandList;
};

#endif
