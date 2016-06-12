#include <string.h>

#include "commands.h"

#include "comm/comm.h"
#include "system.h"
#include "terminal.h"

#include "i2c/i2c.h"
#include "logger/logger.h"

void SendFrameHandler(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);
#if 0
    uint8_t len = strlen(argv[0]);

    CommSendFrame(argv[0], len);
#endif
}

void GetFramesCountHandler(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);
#if 0
    uint8_t count = CommGetFramesCount();

    TerminalPrintf("%d\n", count);
#endif
}

void ReceiveFrameHandler(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);
#if 0
    Frame frame = {.Contents = {0}};
    CommReceiveFrame(&frame);
    CommRemoveFrame();

    TerminalPuts(frame.Contents);
#endif
}

void AutoCommHandlingHandler(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);
}
