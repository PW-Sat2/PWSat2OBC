#include <string.h>
#include <cstdint>
#include <gsl/span>
#include "comm/Beacon.hpp"
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
using std::chrono::seconds;

enum class CommHardware
{
    Hardware = 0,
    Transmitter,
    Receiver,
    Watchdog,
};

static void CommSendFrame(uint16_t argc, char* argv[])
{
    if (argc < 1)
    {
        Main.terminal.Puts("Usage: comm send_frame <content>");
        return;
    }

    uint8_t len = strlen(argv[0]);
    LOGF(LOG_LEVEL_INFO, "Received request to send frame of length %d...", len);
    const auto status = Main.Hardware.CommDriver.SendFrame(span<const uint8_t>(reinterpret_cast<const uint8_t*>(argv[0]), len));
    if (status)
    {
        Main.terminal.Puts("Done");
    }
    else
    {
        Main.terminal.Puts("Failure");
    }
}

static void CommReceiveFrame()
{
    LOG(LOG_LEVEL_INFO, "Received request to get the oldes frame from comm...");
    Frame frame;
    std::uint8_t buffer[PrefferedBufferSize];
    if (!Main.Hardware.CommDriver.ReceiveFrame(buffer, frame))
    {
        LOG(LOG_LEVEL_ERROR, "Unable to get frame from comm. ");
    }
    else
    {
        Main.Hardware.CommDriver.RemoveFrame();
        Main.terminal.PrintBuffer(frame.Payload());
    }
}

static void CommPause()
{
    LOG(LOG_LEVEL_INFO, "Received request to pause comm...");
    if (Main.Hardware.CommDriver.Pause())
    {
        Main.terminal.Puts("Done");
        LOG(LOG_LEVEL_INFO, "Comm paused as requested...");
    }
    else
    {
        Main.terminal.Puts("Failed");
        LOG(LOG_LEVEL_INFO, "Unable to paused comm");
    }
}

static bool GetHardware(const char* parameter, CommHardware& target)
{
    if (strcmp(parameter, "hardware") == 0)
    {
        target = CommHardware::Hardware;
        return true;
    }
    else if (strcmp(parameter, "transmitter") == 0)
    {
        target = CommHardware::Transmitter;
        return true;
    }
    else if (strcmp(parameter, "receiver") == 0)
    {
        target = CommHardware::Receiver;
        return true;
    }
    else if (strcmp(parameter, "watchdog") == 0)
    {
        target = CommHardware::Watchdog;
        return true;
    }
    else
    {
        return false;
    }
}

static void CommReset(uint16_t argc, char* argv[])
{
    static const char* const usage = "comm reset [hardware|transmitter|receiver|watchdog]";
    CommHardware channel;
    if (argc != 1 || !GetHardware(argv[0], channel))
    {
        Main.terminal.Puts(usage);
        return;
    }

    switch (channel)
    {
        case CommHardware::Hardware:
            Main.Hardware.CommDriver.Reset();
            break;

        case CommHardware::Transmitter:
            Main.Hardware.CommDriver.ResetTransmitter();
            break;

        case CommHardware::Receiver:
            Main.Hardware.CommDriver.ResetReceiver();
            break;

        case CommHardware::Watchdog:
            Main.Hardware.CommDriver.ResetWatchdogReceiver();
            break;

        default:
            Main.terminal.Puts(usage);
            break;
    }
}

static void CommGetFrameCount()
{
    LOG(LOG_LEVEL_INFO, "Received request to get the number of received frames from comm...");
    auto count = Main.Hardware.CommDriver.GetFrameCount();
    if (count.status)
    {
        Main.terminal.Printf("%d\n", count.frameCount);
    }
    else
    {
        Main.terminal.Puts("Failed");
    }
}

static void CommGetTelemetry(uint16_t argc, char* argv[])
{
    static const char* const usage = "comm get telemetry [transmitter|receiver]";
    CommHardware channel;
    if (argc != 1 || !GetHardware(argv[0], channel))
    {
        Main.terminal.Puts(usage);
        return;
    }

    if (channel == CommHardware::Transmitter)
    {
        TransmitterTelemetry telemetry;
        if (!Main.Hardware.CommDriver.GetTransmitterTelemetry(telemetry))
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
    else if (channel == CommHardware::Receiver)
    {
        ReceiverTelemetry telemetry;
        if (!Main.Hardware.CommDriver.GetReceiverTelemetry(telemetry))
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
    else
    {
        Main.terminal.Puts(usage);
    }
}

static void CommGetTransmitterState()
{
    TransmitterState state;
    if (!Main.Hardware.CommDriver.GetTransmitterState(state))
    {
        Main.terminal.Puts("Unable to get transmitter state");
        return;
    }

    Main.terminal.Printf("\nStateWhenIdle: %d\nTransmitterBitRate: %d\nBeaconState: %d\n",
        static_cast<int>(state.StateWhenIdle),
        static_cast<int>(state.TransmitterBitRate),
        static_cast<int>(state.BeaconState));
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

static void CommSetBitrate(uint16_t argc, char* argv[])
{
    Bitrate bitRate;
    if (argc != 1 || !GetBitRate(argv[0], bitRate))
    {
        Main.terminal.Puts("comm set bitrate [1200|2400|4800|9600]");
        return;
    }

    if (Main.Hardware.CommDriver.SetTransmitterBitRate(bitRate))
    {
        Main.terminal.Puts("Done");
    }
    else
    {
        Main.terminal.Printf("Unable to set bit rate to: '%d'.", static_cast<int>(bitRate));
    }
}

static void CommSetIdleState(std::uint16_t argc, char* argv[])
{
    if (argc != 1)
    {
        Main.terminal.Puts("comm set idle_state [0|1]");
        return;
    }

    const bool enable = strcmp(argv[0], "1") == 0;
    const auto status = Main.Hardware.CommDriver.SetTransmitterStateWhenIdle(enable ? IdleState::On : IdleState::Off);
    if (status)
    {
        Main.terminal.Puts("Done");
    }
    else
    {
        Main.terminal.Printf("Unable to set transmitter idle state to : '%d'.", static_cast<int>(enable));
    }
}

static void CommSetBeacon(std::uint16_t argc, char* argv[])
{
    if (argc < 3)
    {
        Main.terminal.Puts("comm set beacon <period> <content>");
        return;
    }

    char* tail;
    auto period = strtol(argv[0], &tail, 10);
    auto length = strlen(argv[1]);
    devices::comm::Beacon beacon(seconds(period), gsl::span<const uint8_t>(reinterpret_cast<const uint8_t*>(argv[1]), length));
    auto result = Main.Hardware.CommDriver.SetBeacon(beacon);
    if (!result.HasValue)
    {
        Main.terminal.Puts("Rejected");
    }
    else if (!result.Value)
    {
        Main.terminal.Puts("Failed");
    }
    else
    {
        Main.terminal.Puts("Done");
    }
}

static void CommSet(std::uint16_t argc, char* argv[])
{
    static const char* const usage = "comm set [idle_state|bitrate|beacon]";
    if (argc < 1)
    {
        Main.terminal.Puts(usage);
        return;
    }

    if (strcmp(argv[0], "idle_state") == 0)
    {
        CommSetIdleState(--argc, ++argv);
    }
    else if (strcmp(argv[0], "bitrate") == 0)
    {
        CommSetBitrate(--argc, ++argv);
    }
    else if (strcmp(argv[0], "beacon") == 0)
    {
        CommSetBeacon(--argc, ++argv);
    }
    else
    {
        Main.terminal.Puts(usage);
    }
}

static void CommGet(std::uint16_t argc, char* argv[])
{
    static const char* const usage = "comm get [transmitter_state|telemetry|frame_count]";
    if (argc < 1)
    {
        Main.terminal.Puts(usage);
        return;
    }

    if (strcmp(argv[0], "transmitter_state") == 0)
    {
        CommGetTransmitterState();
    }
    else if (strcmp(argv[0], "telemetry") == 0)
    {
        CommGetTelemetry(--argc, ++argv);
    }
    else if (strcmp(argv[0], "frame_count") == 0)
    {
        CommGetFrameCount();
    }
    else
    {
        Main.terminal.Puts(usage);
    }
}

void Comm(std::uint16_t argc, char* argv[])
{
    static const char* const usage = "comm [set|get|reset|pause|send_frame|receive_frame]";
    if (argc < 1)
    {
        Main.terminal.Puts(usage);
        return;
    }

    if (strcmp(argv[0], "set") == 0)
    {
        CommSet(--argc, ++argv);
    }
    else if (strcmp(argv[0], "get") == 0)
    {
        CommGet(--argc, ++argv);
    }
    else if (strcmp(argv[0], "reset") == 0)
    {
        CommReset(--argc, ++argv);
    }
    else if (strcmp(argv[0], "pause") == 0)
    {
        CommPause();
    }
    else if (strcmp(argv[0], "send_frame") == 0)
    {
        CommSendFrame(--argc, ++argv);
    }
    else if (strcmp(argv[0], "receive_frame") == 0)
    {
        CommReceiveFrame();
    }
    else
    {
        Main.terminal.Puts(usage);
    }
}
