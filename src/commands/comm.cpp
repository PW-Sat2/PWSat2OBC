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
using namespace devices::comm;

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
    std::uint8_t buffer[PrefferedBufferSize];
    if (!Main.Communication.CommDriver.ReceiveFrame(buffer, frame))
    {
        LOG(LOG_LEVEL_ERROR, "Unable to get frame from comm. ");
    }
    else
    {
        Main.Communication.CommDriver.RemoveFrame();

        Main.terminal.PrintBuffer(frame.Payload());
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

void CommGetTelemetry(uint16_t argc, char* argv[])
{
    int channel;
    if (argc != 1 || !GetHardware(argv[0], channel) || (channel != 1 && channel != 2))
    {
        Main.terminal.Puts("comm_get_telemetry [transmitter|receiver]");
        return;
    }

    if (channel == 1)
    {
        TransmitterTelemetry telemetry;
        if (!Main.Communication.CommDriver.GetTransmitterTelemetry(telemetry))
        {
            Main.terminal.Puts("Unable to get transmitter telemetry");
            return;
        }

        Main.terminal.Printf(
            "RFReflectedPower: '%d'\nAmplifierTemperature: '%d'\nRFForwardPower: '%d'\nTransmitterCurrentConsumption: '%d'",
            static_cast<int>(telemetry.RFReflectedPower),
            static_cast<int>(telemetry.AmplifierTemperature),
            static_cast<int>(telemetry.RFForwardPower),
            static_cast<int>(telemetry.TransmitterCurrentConsumption));
    }
    else
    {
        ReceiverTelemetry telemetry;
        if (!Main.Communication.CommDriver.GetReceiverTelemetry(telemetry))
        {
            Main.terminal.Puts("Unable to get receiver telemetry");
            return;
        }

        Main.terminal.Printf("TransmitterCurrentConsumption: '%d'\nReceiverCurrentConsumption: '%d'\nDopplerOffset: '%d'\nVcc: "
                             "'%d'\nOscilatorTemperature: '%d'\nAmplifierTemperature: '%d'\nSignalStrength: '%d'",
            static_cast<int>(telemetry.TransmitterCurrentConsumption),
            static_cast<int>(telemetry.ReceiverCurrentConsumption),
            static_cast<int>(telemetry.DopplerOffset),
            static_cast<int>(telemetry.Vcc),
            static_cast<int>(telemetry.OscilatorTemperature),
            static_cast<int>(telemetry.AmplifierTemperature),
            static_cast<int>(telemetry.SignalStrength));
    }
}

static bool GetBitRate(const char* name, Bitrate& bitRate)
{
    if (strcmp(name, "1200") == 0)
    {
        bitRate = Bitrate::Comm1200bps;
        return true;
    }
    else if (strcmp(name, "2400") == 0)
    {
        bitRate = Bitrate::Comm2400bps;
        return true;
    }
    else if (strcmp(name, "4800") == 0)
    {
        bitRate = Bitrate::Comm4800bps;
        return true;
    }
    else if (strcmp(name, "9600") == 0)
    {
        bitRate = Bitrate::Comm9600bps;
        return true;
    }

    return false;
}

void CommSetBaudRate(uint16_t argc, char* argv[])
{
    Bitrate bitRate;
    if (argc != 1 || !GetBitRate(argv[0], bitRate))
    {
        Main.terminal.Puts("comm_set_bitrate [1200|2400|4800|9600]");
        return;
    }

    if (Main.Communication.CommDriver.SetTransmitterBitRate(bitRate))
    {
        Main.terminal.Puts("Done");
    }
    else
    {
        Main.terminal.Printf("Unable to set bit rate to: '%d'.", static_cast<int>(bitRate));
    }
}

void OBCGetState(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);
    Main.terminal.Printf("%d\n", atomic_load(&Main.initialized) ? 1 : 0);
}
