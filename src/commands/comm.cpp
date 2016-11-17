#include "comm/comm.h"
#include <stdint.h>
#include <string.h>
#include <gsl/span>
#include "commands.h"
#include "logger/logger.h"
#include "obc.h"
#include "system.h"
#include "terminal.h"

using gsl::span;

void SendFrameHandler(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    uint8_t len = strlen(argv[0]);
    LOGF(LOG_LEVEL_INFO, "Received request to send frame of length %d...", len);
    Main.comm.SendFrame(span<const uint8_t>(static_cast<const uint8_t*>(argv[0]), len));
}

void GetFramesCountHandler(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);
    LOG(LOG_LEVEL_INFO, "Received request to get the number of received frames from comm...");
    CommReceiverFrameCount count = Main.comm.GetFrameCount();
    if (count.status)
    {
        TerminalPrintf(&Main.terminal, "%d\n", count.frameCount);
    }
}

void ReceiveFrameHandler(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);
    LOG(LOG_LEVEL_INFO, "Received request to get the oldes frame from comm...");
    CommFrame frame;
    memset(frame.Contents, 0, sizeof(frame.Contents));
    if (!Main.comm.ReceiveFrame(frame))
    {
        LOG(LOG_LEVEL_ERROR, "Unable to get frame from comm. ");
    }
    else
    {
        Main.comm.RemoveFrame();
        TerminalPuts(&Main.terminal, (const char*)frame.Contents);
    }
}

void CommandPauseComm(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);
    LOG(LOG_LEVEL_INFO, "Received request to pause comm...");
    Main.comm.Pause();
    LOG(LOG_LEVEL_INFO, "Comm paused as requested...");
}

void OBCGetState(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);
    TerminalPrintf(&Main.terminal, "%d\n", atomic_load(&Main.initialized) ? 1 : 0);
}
