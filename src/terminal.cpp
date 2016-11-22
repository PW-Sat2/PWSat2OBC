#include "terminal/terminal.h"
#include "commands/commands.h"
#include "obc.h"

typedef void (*commandHandler)(uint16_t argc, char* argv[]);

typedef struct
{
    char name[32];
    commandHandler handler;
} command;

static const TerminalCommandDescription commands[] = {
    {"ping", PingHandler},
    {"echo", EchoHandler},
    {"jumpToTime", JumpToTimeHandler},
    {"currentTime", CurrentTimeHandler},
    {"sendFrame", SendFrameHandler},
    {"getFramesCount", GetFramesCountHandler},
    {"receiveFrame", ReceiveFrameHandler},
    {"pauseComm", CommandPauseComm},
    {"getState", OBCGetState},
    {"listFiles", FSListFiles},
    {"writeFile", FSWriteFile},
    {"readFile", FSReadFile},
    {"stateCmd", CommandByTerminal},
    {"i2c", I2CTestCommandHandler},
    {"heap", HeapInfoCommand} //
};

void InitializeTerminal(void)
{
    TerminalInit(&Main.terminal, &Main.IO);
    TerminalSetCommandList(&Main.terminal, commands, COUNT_OF(commands));
}
