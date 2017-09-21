#ifndef SRC_DEVICES_COMM_H_
#define SRC_DEVICES_COMM_H_

#include <cstdint>
#include <gsl/span>
#include "base/fwd.hpp"
#include "utils.h"

#define COMM_BEGIN                                                                                                                         \
    namespace devices                                                                                                                      \
    {                                                                                                                                      \
        namespace comm                                                                                                                     \
        {
#define COMM_END                                                                                                                           \
    }                                                                                                                                      \
    }
#define COMM devices::comm

COMM_BEGIN

/**
 * @defgroup LowerCommDriver Low Level Communication Module Driver
 * @ingroup device_drivers
 *
 * @brief This module is a low level communication module driver.
 *
 * This driver ensures is responsible for
 *  - direct communication with the hardware,
 *  - hardware state management - i.e. reset on initialization, updating current driver configuration,
 *  - polling for the received frames & dispatching them to the configured component.
 *
 *  The driver works in two basic modes:
 *   - \b Paused - This is the default driver state. In this mode the driver is completely passive and
 *   does not try to monitor the hardware for incoming frames. However it is capable for communicating & directing
 *   the comm hardware to both transmit & receive frames.
 *
 *   - \b Running - This is automatic driver state. In this state the driver itself will try to actively
 *   monitor the hardware for incoming frames download them from the hardware when they are available and
 *   immediately process them. The received frames will be automatically removed from the hardware buffer,
 *   therefore manual frame erasure is not recommended at it will lead to lost frames.
 *
 *  @{
 */

/**
 * @defgroup LowerCommDriverLowInterface Low Level interface
 *
 * This is low level API intended only for use when the driver is in passive mode.
 */

class Frame;
class Beacon;
class CommTelemetry;
class CommObject;

struct IHandleFrame;
struct ITransmitter;
struct IBeaconController;
struct ICommTelemetryProvider;

/**
 * @brief Maximum allowed single frame content length.
 */
constexpr std::uint16_t MaxDownlinkFrameSize = 235u;

/** @brief Maximum size of uplink frame */
constexpr std::uint16_t MaxUplinkFrameSize = 200u;

/**
 * @brief Maximum allowed single frame content length.
 */
constexpr std::uint16_t PrefferedBufferSize = MaxDownlinkFrameSize + 20;

/** Transmitter state enumerator. */
enum class IdleState
{
    /** Disabled. */
    Off = 0,

    /** Enabled. */
    On = 1,
};

/** Transmission baud rate enumerator. */
enum class Bitrate
{
    /** 1200 bps */
    Comm1200bps = 1,

    /** 2400 bps */
    Comm2400bps = 2,

    /** 4800 bps */
    Comm4800bps = 4,

    /** 9600 bps */
    Comm9600bps = 8,
};

/**
 * @brief This type contains comm receiver telemetry.
 *
 * The values stored in this structure are provided in their raw form
 * as they are received directly from the hardware.
 */
struct ReceiverTelemetry
{
    std::chrono::seconds Uptime;

    std::uint16_t LastReceivedDopplerOffset;
    std::uint16_t LastReceivedRSSI;

    std::uint16_t NowDopplerOffset;
    std::uint16_t NowReceiverCurrentConsumption;
    std::uint16_t NowVoltage;
    std::uint16_t NowOscilatorTemperature;
    std::uint16_t NowAmplifierTemperature;
    std::uint16_t NowRSSI;

    static constexpr std::size_t BitSize();
};

constexpr std::size_t ReceiverTelemetry::BitSize()
{
    return 113;
}

/**
 * @brief This type contains comm transmitter telemetry.
 *
 * The values stored in this structure are provided in their raw form
 * as they are received directly from the hardware.
 */
struct TransmitterTelemetry
{
    std::chrono::seconds Uptime;

    Bitrate TransmitterBitRate;

    std::uint16_t LastTransmittedRFReflectedPower;
    std::uint16_t LastTransmittedAmplifierTemperature;
    std::uint16_t LastTransmittedRFForwardPower;
    std::uint16_t LastTransmittedTransmitterCurrentConsumption;

    std::uint16_t NowRFForwardPower;
    std::uint16_t NowTransmitterCurrentConsumption;

    IdleState StateWhenIdle;

    bool BeaconState;

    static constexpr std::size_t BitSize();
};

constexpr std::size_t TransmitterTelemetry::BitSize()
{
    return 93;
}

/** Type that contains status of the frame count query. */
struct ReceiverFrameCount
{
    /** @brief Frame count query status. */
    bool status;

    /**
     * @brief Number of the available frames in the frame buffer.
     *
     * The contents of this field is undefined when the status is set to false.
     */
    std::uint16_t frameCount;
};

/**
 * @brief Enumerator for all supported comm frame receiver commands.
 */
enum class ReceiverCommand
{
    SoftReset = 0xAA,
    HardReset = 0xAB,
    ResetWatchdog = 0xCC,
    GetUptime = 0x40,
    GetFrameCount = 0x21,
    GetFrame = 0x22,
    RemoveFrame = 0x24,
    GetTelemetry = 0x1A,
};

/**
 * @brief Enumerator for all supported comm frame transmitter commands.
 */
enum class TransmitterCommand
{
    SoftReset = 0xAA,
    HardReset = 0xAB,
    ResetWatchdog = 0xCC,
    SendFrame = 0x10,
    GetTelemetryLastTransmission = 0x26,
    GetTelemetryInstant = 0x25,
    SetBeacon = 0x14,
    ClearBeacon = 0x1f,
    SetIdleState = 0x24,
    SetBitRate = 0x28,
    GetUptime = 0x40,
    GetState = 0x41
};

/**
 * @brief Enumerator of all communication hardware addresses.
 */
enum class Address
{
    Receiver = 0x60,
    Transmitter = 0x61,
};

/**
 * @brief Interface of object capable providing complete comm telemetry.
 */
struct ICommTelemetryProvider
{
    /**
     * @brief Acquires complete current comm hardware telemetry.
     * @param[out] telemetry Object that should be filled with updated telemetry.
     * @return Operation status. True on success, false otherwise.
     */
    virtual bool GetTelemetry(CommTelemetry& telemetry) = 0;
};

/**
 * @brief Interface of object that is periodically querying the comm hardware for incoming frames.
 */
struct ICommHardwareObserver
{
    /**
     * @brief Wait for the next comm hardware polling iteration.
     */
    virtual void WaitForComLoop() = 0;
};

/** @}*/
COMM_END

#endif /* SRC_DEVICES_COMM_H_ */
