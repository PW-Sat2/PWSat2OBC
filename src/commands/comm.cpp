#include "comm/comm.h"
#include <string.h>
#include <cstdint>
#include <gsl/span>
#include <gsl/span>
#include "commands.h"
#include "logger/logger.h"
#include "obc.h"
#include "system.h"
#include "terminal.h"

using std::uint16_t;
using std::uint8_t;
using gsl::span;
using devices::comm::CommReceiverFrameCount;
using devices::comm::CommFrame;

void SendFrameHandler(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    uint8_t len = strlen(argv[0]);
    LOGF(LOG_LEVEL_INFO, "Received request to send frame of length %d...", len);
    Main.Communication.CommDriver.SendFrame(span<const uint8_t>(reinterpret_cast<const uint8_t*>(argv[0]), len));
}

void GetFramesCountHandler(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);
    LOG(LOG_LEVEL_INFO, "Received request to get the number of received frames from comm...");
    CommReceiverFrameCount count = Main.Communication.CommDriver.GetFrameCount();
    if (count.status)
    {
        Main.terminal.Printf("%d\n", count.frameCount);
    }
}

void ReceiveFrameHandler(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);
    LOG(LOG_LEVEL_INFO, "Received request to get the oldes frame from comm...");
    CommFrame frame;
    if (!Main.Communication.CommDriver.ReceiveFrame(frame))
    {
        LOG(LOG_LEVEL_ERROR, "Unable to get frame from comm. ");
    }
    else
    {
        Main.Communication.CommDriver.RemoveFrame();

        auto payload = frame.Payload();
        auto payloadStr = reinterpret_cast<const char*>(&payload[0]);

        Main.terminal.PrintBuffer(gsl::span<const char>(payloadStr, frame.Size));
    }
}

void CommandPauseComm(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);
    LOG(LOG_LEVEL_INFO, "Received request to pause comm...");
    Main.Communication.CommDriver.Pause();
    LOG(LOG_LEVEL_INFO, "Comm paused as requested...");
}

void OBCGetState(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);
    Main.terminal.Printf("%d\n", atomic_load(&Main.initialized) ? 1 : 0);
}
