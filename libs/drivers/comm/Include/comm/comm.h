#ifndef SRC_DEVICES_COMM_H_
#define SRC_DEVICES_COMM_H_

#include <stdbool.h>
#include <stdint.h>
#include <em_i2c.h>

#include "base/os.h"
#include "i2c/i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup LowerCommDriver Low Level Communication Module Driver
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

/**
 * @brief Maximum allowed single frame content length.
 */
#define COMM_MAX_FRAME_CONTENTS_SIZE 235u

/** @brief This type describe comm driver global state. */
class CommObject;

/**
 * @brief This type describes single received frame.
 */
typedef struct
{
    /** @brief Frame contents size in bytes. */
    uint16_t Size;

    /** @brief Doppler frequency. This field contains the measured Doppler shift on the packet at the reception time. */
    uint16_t Doppler;

    /** @brief This field contains the measured Received Signal Strength Indicator (RSSI) at the reception time. */
    uint16_t RSSI;

    /** @brief Frame content. */
    uint8_t Contents[COMM_MAX_FRAME_CONTENTS_SIZE];
} CommFrame;

/**
 * @brief This type contains comm receiver telemetry.
 *
 * The values stored in this structure are provided in their raw form
 * as they are received directly from the hardware.
 */
typedef struct
{
    /** @brief Raw measurement value of the transmitter current consumption. */
    uint16_t TransmitterCurrentConsumption;

    /** @brief Raw measurement value of the receiver current consumption. */
    uint16_t ReceiverCurrentConsumption;

    /** @brief Raw measurement value of the instantaneous Doppler offset of the signal at the receiver port. */
    uint16_t DopplerOffset;

    /** @brief Raw measurement value of the power bus voltage.*/
    uint16_t Vcc;

    /** @brief Raw measurement value of the local oscillator temperature. */
    uint16_t OscilatorTemperature;

    /** @brief Raw measurement value of the power amplifier temperature. */
    uint16_t AmplifierTemperature;

    /** @brief Raw measurement value of the instantaneous signal strength of the signal at the receiver. */
    uint16_t SignalStrength;
} CommReceiverTelemetry;

/**
 * @brief This type contains comm transmitter telemetry.
 *
 * The values stored in this structure are provided in their raw form
 * as they are received directly from the hardware.
 */
typedef struct
{
    /** @brief Raw measurement value of the instantaneous RF reflected power at the transmitter port. */
    uint16_t RFReflectedPower;

    /** @brief Raw measurement value of the power amplifier temperature. */
    uint16_t AmplifierTemperature;

    /** @brief Raw measurement value of the instantaneous RF forward power at the transmitter port. */
    uint16_t RFForwardPower;

    /** @brief Raw measurement value of the transmitter current consumption. */
    uint16_t TransmitterCurrentConsumption;
} CommTransmitterTelemetry;

/**
 * @brief This type describes the comm beacon.
 */
typedef struct
{
    /**
     * @brief Repeat interval of the beacon in seconds.
     *
     * This is the interval between the starts of two transmissions of the beacon. This interval
     * has a minimum value of 0 and a maximum value of 3000. If an interval of more than 3000 seconds
     * is specified, the interval will automatically be set to 3000.
     */
    uint16_t Period;

    /** @brief Beacon frame contents size in bytes. */
    uint8_t DataSize;

    /** @brief Beacon frame contents. */
    uint8_t Data[COMM_MAX_FRAME_CONTENTS_SIZE];
} CommBeacon;

/**
 * @brief Type of pointer to function that is capable of processing frames received via the
 * comm receiver.
 *
 * @param[in] comm Pointer to comm object that received the frame.
 * @param[in] frame Pointer to object describing received frame.
 * @param[in] context Pointer to user supplied memory block.
 * The comm driver does not use this parameter. It is provided on comm initialization by the
 * user and is passed to the frame handler routine in unmodified form.
 */
typedef void (*CommFrameHandler)(CommObject* comm, CommFrame* frame, void* context);

/** Comm driver upper interface. */
struct IHandleFrame
{
    /**
     * @brief Pointer to function that should be called whenever new frame is received.
     *
     * See the CommFrameHandler definition for details regarding usage & implementation requirements.
     */
    virtual void HandleFrame(CommObject* comm, CommFrame* frame, void* context) = 0;
};

/** Type that contains status of the frame count query. */
struct CommReceiverFrameCount
{
    /** @brief Frame count query status. */
    bool status;

    /**
     * @brief Number of the available frames in the frame buffer.
     *
     * The contents of this field is undefined when the status is set to false.
     */
    uint8_t frameCount;
};

/** Transmitter state enumerator. */
typedef enum {
    /** Disabled. */
    CommTransmitterOff = 0,

    /** Enabled. */
    CommTransmitterOn = 1,
} CommTransmitterIdleState;

/** Transmission baud rate enumerator. */
typedef enum {
    /** 1200 bps */
    Comm1200bps = 1,

    /** 2400 bps */
    Comm2400bps = 2,

    /** 4800 bps */
    Comm4800bps = 4,

    /** 9600 bps */
    Comm9600bps = 8,
} CommTransmitterBitrate;

/** This type describes configured transmitter state. */
typedef struct
{
    /** The transmitter state when there are no frames to send. */
    CommTransmitterIdleState StateWhenIdle;

    /** Transmission speed. */
    CommTransmitterBitrate TransmitterBitRate;

    /** Flag indicating whether beacon is active. */
    bool BeaconState;
} CommTransmitterState;

/**
 * @brief This type describe comm driver global state.
 *
 * @remark Do not access directly the fields of this type, instead use the comm driver interface to
 * perform requested action.
 */
class CommObject final
{
  public:
    CommObject(I2CBus& low, IHandleFrame& upperInterface);

    /**
     * @brief This procedure initializes the comm driver object and sets it 'Paused' state.
     *
     * @return Operation status.
     *
     * This procedure does not verify whether the passed comm object has already been properly initialized, calling
     * this method twice on the same comm object is undefined behavior.
     */
    OSResult Initialize();

    /**
     * @brief Pauses comm driver.
     *
     * This procedure suspends execution of the comm driver background tasks and moves that driver itself to 'Paused'
     * (passive) state. This method will not stop already ongoing communication with the hardware.
     *
     * Calling this method twice without intermediate call to CommRestart causes caller to hang until someone else
     * resumes the driver.
     *
     * @return Operation status, true in case of success, false otherwise.
     */
    bool Pause();

    /**
     * @brief Restarts the comm driver.
     *
     * @return Operation status, true in case of success, false otherwise.
     *
     * During the driver restart process entire hardware is being reseted, and all of the background
     * tasks are being started.
     *
     * Calling this method twice without intermediate call to the CommPause procedure leads to undefined behavior.
     */
    bool Restart();

    /**
     * @brief Queries comm driver for a number of received and not yet processed frames.
     *
     * @return Operation status. See the definition of the CommReceiverFrameCount for details.
     */
    CommReceiverFrameCount GetFrameCount();

    /**
     * @brief Removes the oldest frame from the input frame buffer.
     *
     * @return Operation status, true in case of success, false otherwise.
     */
    bool RemoveFrame();

    /**
     * @brief Queries the comm driver for the receiver telemetry.
     *
     * @param[out] telemetry Pointer to object that should be filled with current receiver telemetry.
     * @return Operation status, true in case of success, false otherwise.
     *
     * The contents of the telemetry object is undefined in case of the failure.
     */
    bool GetReceiverTelemetry(CommReceiverTelemetry* telemetry);

    /**
     * @brief Queries the comm driver for the transmitter telemetry.
     *
     * @param[out] telemetry Pointer to object that should be filled with current transmitter telemetry.
     * @return Operation status, true in case of success, false otherwise.
     *
     * The contents of the telemetry object is undefined in case of the failure.
     */
    bool GetTransmitterTelemetry(CommTransmitterTelemetry* telemetry);

    /**
     * @brief Adds the requested frame to the send queue.
     *
     * @param[in] data Pointer to buffer that contains the frame contents.
     * @param[in] length Size of the frame contents in bytes. This value cannot be
     * greater then COMM_MAX_FRAME_CONTENTS_SIZE.
     * @return Operation status, true in case of success, false otherwise.
     */
    bool SendFrame(uint8_t* data, uint8_t length);

    /**
     * @brief Requests the contents of the oldest received frame from the queue.
     *
     * @param[out] frame Pointer to object that should be filled with the data describing the oldest
     * available received frame.
     * @return Operation status, true in case of success, false otherwise.
     *
     * The contents of the frame object is undefined in case of the failure.
     */
    bool ReceiveFrame(CommFrame* frame);

    /**
     * @brief This procedure sets the beacon frame for the passed comm object.
     *
     * @param[in] beaconData Pointer to object describing new beacon.
     * See the definition of the CommBeacon for details.
     * @return Operation status, true in case of success, false otherwise.
     */
    bool SetBeacon(const CommBeacon* beaconData);

    /**
     * @brief Clears any beacon that is currently set in the transceiver. If a beacon transmission
     * is currently in progress, this transmission will be completed.
     *
     * @return Operation status, true in case of success, false otherwise.
     */
    bool ClearBeacon();

    /**
     * @brief Set the transmitter state when there are no more frames to sent.
     *
     * @param[in] requestedState New transmitter state when it is idle.
     * @return Operation status, true in case of success, false otherwise.
     */
    bool SetTransmitterStateWhenIdle(CommTransmitterIdleState requestedState);

    /**
     * @brief Set the transmitter baud rate.
     *
     * @param[in] bitrate New transmitter baud rate.
     * @return Operation status, true in case of success, false otherwise.
     */
    bool SetTransmitterBitRate(CommTransmitterBitrate bitrate);

    /**
     * @brief Queries the comm driver object for current transmitter state.
     *
     * @param[out] state Pointer to object that should be filled with the data describing
     * the current transmitter state.
     * @return Operation status, true in case of success, false otherwise.
     *
     * The contents of the state object is undefined in case of the failure.
     */
    bool GetTransmitterState(CommTransmitterState* state);

    /**
     * @brief Resets the hardware associated with the requested comm object.
     *
     * @return Operation status, true in case of success, false otherwise.
     */
    bool Reset();

    /**
     * @brief Resets the transmitter hardware associated with the requested comm object.
     *
     * @return Operation status, true in case of success, false otherwise.
     */
    bool ResetTransmitter();

    /**
     * @brief Resets the receiver hardware associated with the requested comm object.
     *
     * @return Operation status, true in case of success, false otherwise.
     */
    bool ResetReceiver();

    /** @brief Comm driver lower interface. */
    I2CBus& low;

    /** @brief Comm driver upper interface. */
    IHandleFrame& upper;

    /** @brief Handle to comm background task. */
    void* commTask;

    /** @brief Handle to event group used to communicate with background task. */
    OSEventGroupHandle commTaskFlags;
};

/** @}*/

#ifdef __cplusplus
}
#endif

#endif /* SRC_DEVICES_COMM_H_ */
