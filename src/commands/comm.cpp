#include <string.h>
#include <cstdint>
#include <cstdlib>
#include <gsl/span>
#include "comm/Beacon.hpp"
#include "comm/CommDriver.hpp"
#include "comm/Frame.hpp"
#include "commands.h"
#include "logger/logger.h"
#include "obc_access.hpp"
#include "system.h"
#include "terminal.h"
#include "terminal/terminal.h"

using std::uint16_t;
using std::uint8_t;
using gsl::span;
using namespace devices::comm;
using std::chrono::seconds;
using devices::comm::IdleState;

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
        GetTerminal().Puts("Usage: comm send_frame <content>");
        return;
    }

    uint8_t len = strlen(argv[0]);
    LOGF(LOG_LEVEL_INFO, "Received request to send frame of length %d...", len);
    const auto status = GetCommDriver().SendFrame(span<const uint8_t>(reinterpret_cast<const uint8_t*>(argv[0]), len));
    if (status)
    {
        GetTerminal().Puts("Done");
    }
    else
    {
        GetTerminal().Puts("Failure");
    }
}

static void CommReceiveFrame()
{
    LOG(LOG_LEVEL_INFO, "Received request to get the oldes frame from comm...");
    Frame frame;
    std::uint8_t buffer[PrefferedBufferSize];
    if (!GetCommDriver().ReceiveFrame(buffer, frame))
    {
        LOG(LOG_LEVEL_ERROR, "Unable to get frame from comm. ");
    }
    else
    {
        GetCommDriver().RemoveFrame();
        GetTerminal().PrintBuffer(frame.Payload());
    }
}

static void CommPause()
{
    LOG(LOG_LEVEL_INFO, "Received request to pause comm...");
    if (GetCommDriver().Pause())
    {
        GetTerminal().Puts("Done");
        LOG(LOG_LEVEL_INFO, "Comm paused as requested...");
    }
    else
    {
        GetTerminal().Puts("Failed");
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
        GetTerminal().Puts(usage);
        return;
    }

    switch (channel)
    {
        case CommHardware::Hardware:
            GetCommDriver().Reset();
            break;

        case CommHardware::Transmitter:
            GetCommDriver().ResetTransmitter();
            break;

        case CommHardware::Receiver:
            GetCommDriver().ResetReceiver();
            break;

        case CommHardware::Watchdog:
            GetCommDriver().ResetWatchdogReceiver();
            break;

        default:
            GetTerminal().Puts(usage);
            break;
    }
}

static void CommGetFrameCount()
{
    LOG(LOG_LEVEL_INFO, "Received request to get the number of received frames from comm...");
    auto count = GetCommDriver().GetFrameCount();
    if (count.status)
    {
        GetTerminal().Printf("%d\n", count.frameCount);
    }
    else
    {
        GetTerminal().Puts("Failed");
    }
}

static void CommGetTelemetry(uint16_t argc, char* argv[])
{
    static const char* const usage = "comm get telemetry [transmitter|receiver]";
    CommHardware channel;
    if (argc != 1 || !GetHardware(argv[0], channel))
    {
        GetTerminal().Puts(usage);
        return;
    }

    if (channel == CommHardware::Transmitter)
    {
        TransmitterTelemetry telemetry;
        if (!GetCommDriver().GetTransmitterTelemetry(telemetry))
        {
            GetTerminal().Puts("Unable to get transmitter telemetry");
            return;
        }

        GetTerminal().Printf("Uptime: '%ld'\n", static_cast<std::uint32_t>(telemetry.Uptime.count()));
        GetTerminal().Printf("Bitrate: '%d'\n", num(telemetry.TransmitterBitRate));
        GetTerminal().Printf("LastTransmitted RF Reflected power: '%d'\n", telemetry.LastTransmittedRFReflectedPower.Value());
        GetTerminal().Printf("LastTransmitted Power Amp Temperature: '%d'\n", telemetry.LastTransmittedAmplifierTemperature.Value());
        GetTerminal().Printf("LastTransmitted RF Forward power: '%d'\n", telemetry.LastTransmittedRFForwardPower.Value());
        GetTerminal().Printf("LastTransmitted TX Current: '%d'\n", telemetry.LastTransmittedTransmitterCurrentConsumption.Value());

        GetTerminal().Printf("Now RF Forward power: '%d'\n", telemetry.NowRFForwardPower.Value());
        GetTerminal().Printf("Now TX Current: '%d'\n", telemetry.NowTransmitterCurrentConsumption.Value());

        GetTerminal().Printf("Idle state: '%d'\n", telemetry.StateWhenIdle == IdleState::On ? 1 : 0);
        GetTerminal().Printf("Beacon: '%d'\n", telemetry.BeaconState ? 1 : 0);
    }
    else if (channel == CommHardware::Receiver)
    {
        ReceiverTelemetry telemetry;
        if (!GetCommDriver().GetReceiverTelemetry(telemetry))
        {
            GetTerminal().Puts("Unable to get receiver telemetry");
            return;
        }

        GetTerminal().Printf("Uptime: '%ld'\n", static_cast<std::uint32_t>(telemetry.Uptime.count()));
        GetTerminal().Printf("LastReceived Doppler: '%d'\n", telemetry.LastReceivedDopplerOffset.Value());
        GetTerminal().Printf("LastReceived RSSI: '%d'\n", telemetry.LastReceivedRSSI.Value());

        GetTerminal().Printf("Now Doppler: '%d'\n", telemetry.NowDopplerOffset.Value());
        GetTerminal().Printf("Now RX current: '%d'\n", telemetry.NowReceiverCurrentConsumption.Value());
        GetTerminal().Printf("Now Power Supply Voltage: '%d'\n", telemetry.NowVoltage.Value());
        GetTerminal().Printf("Now Oscillator Temperature: '%d'\n", telemetry.NowOscilatorTemperature.Value());
        GetTerminal().Printf("Now Power Amp Temperature: '%d'\n", telemetry.NowAmplifierTemperature.Value());
        GetTerminal().Printf("Now RSSI: '%d'\n", telemetry.NowRSSI.Value());
    }
    else
    {
        GetTerminal().Puts(usage);
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

static void CommSetBitrate(uint16_t argc, char* argv[])
{
    Bitrate bitRate;
    if (argc != 1 || !GetBitRate(argv[0], bitRate))
    {
        GetTerminal().Puts("comm set bitrate [1200|2400|4800|9600]");
        return;
    }

    if (GetCommDriver().SetTransmitterBitRate(bitRate))
    {
        GetTerminal().Puts("Done");
    }
    else
    {
        GetTerminal().Printf("Unable to set bit rate to: '%d'.", static_cast<int>(bitRate));
    }
}

static void CommSetIdleState(std::uint16_t argc, char* argv[])
{
    if (argc != 1)
    {
        GetTerminal().Puts("comm set idle_state [0|1]");
        return;
    }

    const bool enable = strcmp(argv[0], "1") == 0;
    const auto status = GetCommDriver().SetTransmitterStateWhenIdle(enable ? IdleState::On : IdleState::Off);
    if (status)
    {
        GetTerminal().Puts("Done");
    }
    else
    {
        GetTerminal().Printf("Unable to set transmitter idle state to : '%d'.", static_cast<int>(enable));
    }
}

static void CommSetBeacon(std::uint16_t argc, char* argv[])
{
    if (argc < 3)
    {
        GetTerminal().Puts("comm set beacon <period> <content>");
        return;
    }

    char* tail;
    auto period = strtol(argv[0], &tail, 10);
    auto length = strlen(argv[1]);
    devices::comm::Beacon beacon(seconds(period), gsl::span<const uint8_t>(reinterpret_cast<const uint8_t*>(argv[1]), length));
    auto result = GetCommDriver().SetBeacon(beacon);
    if (!result.HasValue)
    {
        GetTerminal().Puts("Rejected");
    }
    else if (!result.Value)
    {
        GetTerminal().Puts("Failed");
    }
    else
    {
        GetTerminal().Puts("Done");
    }
}

static void CommSet(std::uint16_t argc, char* argv[])
{
    static const char* const usage = "comm set [idle_state|bitrate|beacon]";
    if (argc < 1)
    {
        GetTerminal().Puts(usage);
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
        GetTerminal().Puts(usage);
    }
}

static void CommGet(std::uint16_t argc, char* argv[])
{
    static const char* const usage = "comm get [transmitter_state|telemetry|frame_count]";
    if (argc < 1)
    {
        GetTerminal().Puts(usage);
        return;
    }

    if (strcmp(argv[0], "telemetry") == 0)
    {
        CommGetTelemetry(--argc, ++argv);
    }
    else if (strcmp(argv[0], "frame_count") == 0)
    {
        CommGetFrameCount();
    }
    else
    {
        GetTerminal().Puts(usage);
    }
}

void Comm(std::uint16_t argc, char* argv[])
{
    static const char* const usage = "comm [set|get|reset|pause|send_frame|receive_frame]";
    if (argc < 1)
    {
        GetTerminal().Puts(usage);
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
        GetTerminal().Puts(usage);
    }
}
