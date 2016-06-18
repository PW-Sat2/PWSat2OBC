#include "comm/comm.h"
#include <stdint.h>
#include <string.h>
#include "commands.h"
#include "logger/logger.h"
#include "obc.h"
#include "system.h"
#include "terminal.h"

void SendFrameHandler(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    uint8_t len = strlen(argv[0]);
    CommSendFrame(&Main.comm, (uint8_t*)argv[0], len);
}

void GetFramesCountHandler(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);
    LOG(LOG_LEVEL_INFO, "Received request to get the number of received frames from comm...");
    CommReceiverFrameCount count = CommGetFrameCount(&Main.comm);
    if (count.status)
    {
        TerminalPrintf("%d\n", count.frameCount);
    }
}

void ReceiveFrameHandler(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);
    LOG(LOG_LEVEL_INFO, "Received request to get the oldes frame from comm...");
    Frame frame = {.Contents = {0}};
    if (!CommReceiveFrame(&Main.comm, &frame))
    {
        LOG(LOG_LEVEL_ERROR, "Unable to get frame from comm. ");
    }
    else
    {
        CommRemoveFrame(&Main.comm);
        TerminalPuts((const char*)frame.Contents);
    }
}

void CommandPauseComm(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);
    LOG(LOG_LEVEL_INFO, "Received request to pause comm...");
    CommPause(&Main.comm);
}

void OBCGetState(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);
    TerminalPrintf("%d\n", atomic_load(&Main.initialized) ? 1 : 0);
}
