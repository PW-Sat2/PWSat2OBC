#include <string.h>

#include "commands.h"

#include "devices/comm.h"
#include "system.h"
#include "terminal.h"

void SendFrameHandler(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);

    uint8_t len = strlen(argv[0]);

    CommSendFrame(argv[0], len);
}

void GetFramesCountHandler(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    uint8_t count = CommGetFramesCount();

    TerminalPrintf("%d\n", count);
}

void ReceiveFrameHandler(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    Frame frame = {.Contents = {0}};
    CommReceiveFrame(&frame);
    CommRemoveFrame();

    TerminalPuts(frame.Contents);
}
