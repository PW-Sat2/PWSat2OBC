#ifndef SRC_TERMINAL_H_
#define SRC_TERMINAL_H_

#include <stdint.h>
#include <gsl/span>
#include "base/os.h"
#include "leuart/line_io.h"

using TerminalCommandHandler = void (*)(uint16_t argc, char* argv[]);

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
    Task<Terminal*, void (*)(Terminal*), 2500, 4> task;

    gsl::span<const TerminalCommandDescription> commandList;
};

#endif
