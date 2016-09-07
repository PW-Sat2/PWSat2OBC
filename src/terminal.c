#include "terminal/terminal.h"
#include "commands/commands.h"
#include "obc.h"

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
    {"i2c", I2CTestCommandHandler} //
};

void InitializeTerminal(void)
{
    TerminalInit(&Main.terminal, &Main.IO);
    TerminalSetCommandList(&Main.terminal, commands, COUNT_OF(commands));
}
