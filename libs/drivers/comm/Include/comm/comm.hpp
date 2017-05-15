#ifndef SRC_DEVICES_COMM_H_
#define SRC_DEVICES_COMM_H_

#include <cstdint>
#include <gsl/span>
#include "base/fwd.hpp"

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

/**
 * @brief This type contains comm receiver telemetry.
 *
 * The values stored in this structure are provided in their raw form
 * as they are received directly from the hardware.
 */
struct ReceiverTelemetry
{
    /** @brief Raw measurement value of the transmitter current consumption. */
    std::uint16_t TransmitterCurrentConsumption;

    /** @brief Raw measurement value of the receiver current consumption. */
    std::uint16_t ReceiverCurrentConsumption;

    /** @brief Raw measurement value of the instantaneous Doppler offset of the signal at the receiver port. */
    std::uint16_t DopplerOffset;

    /** @brief Raw measurement value of the power bus voltage.*/
    std::uint16_t Vcc;

    /** @brief Raw measurement value of the local oscillator temperature. */
    std::uint16_t OscilatorTemperature;

    /** @brief Raw measurement value of the power amplifier temperature. */
    std::uint16_t AmplifierTemperature;

    /** @brief Raw measurement value of the instantaneous signal strength of the signal at the receiver. */
    std::uint16_t SignalStrength;
};

/**
 * @brief This type contains comm transmitter telemetry.
 *
 * The values stored in this structure are provided in their raw form
 * as they are received directly from the hardware.
 */
struct TransmitterTelemetry
{
    /** @brief Raw measurement value of the instantaneous RF reflected power at the transmitter port. */
    std::uint16_t RFReflectedPower;

    /** @brief Raw measurement value of the power amplifier temperature. */
    std::uint16_t AmplifierTemperature;

    /** @brief Raw measurement value of the instantaneous RF forward power at the transmitter port. */
    std::uint16_t RFForwardPower;

    /** @brief Raw measurement value of the transmitter current consumption. */
    std::uint16_t TransmitterCurrentConsumption;
};

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

/** This type describes configured transmitter state. */
struct TransmitterState
{
    /** The transmitter state when there are no frames to send. */
    IdleState StateWhenIdle;

    /** Transmission speed. */
    Bitrate TransmitterBitRate;

    /** Flag indicating whether beacon is active. */
    bool BeaconState;
};

/**
 * @brief Enumerator for all supported comm frame receiver commands.
 */
enum class ReceiverCommand
{
    SoftReset = 0xAA,
    HardReset = 0xAB,
    ResetWatchdog = 0xCC,
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
    GetTelemetry = 0x26,
    SetBeacon = 0x14,
    ClearBeacon = 0x1f,
    SetIdleState = 0x24,
    SetBitRate = 0x28,
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

struct ICommTelemetryProvider
{
    virtual bool GetTelemetry(CommTelemetry& telemetry) = 0;
};
/** @}*/
COMM_END

#endif /* SRC_DEVICES_COMM_H_ */
