#include <string.h>
#include <cstdint>
#include <gsl/span>
#include <gsl/span>
#include "comm/Frame.hpp"
#include "commands.h"
#include "logger/logger.h"
#include "obc.h"
#include "system.h"
#include "terminal.h"

using std::uint16_t;
using std::uint8_t;
using gsl::span;
using devices::comm::Frame;

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
    auto count = Main.Communication.CommDriver.GetFrameCount();
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
    Frame frame;
    std::uint8_t buffer[devices::comm::PrefferedBufferSize];
    if (!Main.Communication.CommDriver.ReceiveFrame(buffer, frame))
    {
        LOG(LOG_LEVEL_ERROR, "Unable to get frame from comm. ");
    }
    else
    {
        Main.Communication.CommDriver.RemoveFrame();

        auto payload = frame.Payload();
        auto payloadStr = reinterpret_cast<const char*>(payload.data());
        Main.terminal.PrintBuffer(gsl::span<const char>(payloadStr, payload.size()));
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

static bool GetHardware(const char* parameter, int& target)
{
    if (strcmp(parameter, "hardware") == 0)
    {
        target = 0;
        return true;
    }
    else if (strcmp(parameter, "transmitter") == 0)
    {
        target = 1;
        return true;
    }
    else if (strcmp(parameter, "receiver") == 0)
    {
        target = 2;
        return true;
    }
    else if (strcmp(parameter, "watchdog") == 0)
    {
        target = 3;
        return true;
    }
    else
    {
        return false;
    }
}

void CommReset(uint16_t argc, char* argv[])
{
    int channel;
    if (argc != 1 || !GetHardware(argv[0], channel))
    {
        Main.terminal.Puts("comm_reset [hardware|transmitter|receiver|watchdog]");
        return;
    }

    if (channel == 0)
    {
        Main.Communication.CommDriver.Reset();
    }
    else if (channel == 1)
    {
        Main.Communication.CommDriver.ResetTransmitter();
    }
    else if (channel == 2)
    {
        Main.Communication.CommDriver.ResetReceiver();
    }
    else
    {
        Main.Communication.CommDriver.ResetWatchdogReceiver();
    }
}

void OBCGetState(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);
    Main.terminal.Printf("%d\n", atomic_load(&Main.initialized) ? 1 : 0);
}
