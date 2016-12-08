#include <gsl/span>

#include "commands/commands.h"
#include "obc.h"
#include "terminal/terminal.h"

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
    {"mountList", MountList},
    {"mkdir", MakeDirectory},
    {"i2c", I2CTestCommandHandler},
    {"antenna_deploy", AntennaDeploy},
    {"antenna_cancel", AntennaCancelDeployment},
    {"antenna_get_status", AntennaGetDeploymentStatus},
    {"heap", HeapInfo},
    {"tasklist", TaskListCommand},
    {"heap", HeapInfoCommand},
    {"advance_time", AdvanceTimeHandler},
    {"compile_info", CompileInfo},
};

void InitializeTerminal(void)
{
    Main.terminal.Initialize();
    Main.terminal.SetCommandList(gsl::span<const TerminalCommandDescription>(commands));
}
