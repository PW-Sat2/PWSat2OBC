#ifndef LIBS_DRIVERS_COMM_COMM_DRIVER_HPP
#define LIBS_DRIVERS_COMM_COMM_DRIVER_HPP

#pragma once

#include "ITransmitFrame.hpp"
#include "base/os.h"
#include "comm.hpp"
#include "i2c/forward.h"

COMM_BEGIN

/**
 * @brief This type describe comm driver global state.
 * @ingroup LowerCommDriver
 * @remark Do not access directly the fields of this type, instead use the comm driver interface to
 * perform requested action.
 */
class CommObject final : public ITransmitFrame
{
  public:
    /**
     * Constructs new instance of COMM low-level driver
     * @param[in] low I2C bus used to communicate with device
     * @param[in] upperInterface Reference to object responsible for interpreting received frames
     */
    CommObject(drivers::i2c::II2CBus& low, IHandleFrame& upperInterface);

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
    ReceiverFrameCount GetFrameCount();

    /**
     * @brief Removes the oldest frame from the input frame buffer.
     *
     * @return Operation status, true in case of success, false otherwise.
     */
    bool RemoveFrame();

    /**
     * @brief Queries the comm driver for the receiver telemetry.
     *
     * @param[out] telemetry Reference to object that should be filled with current receiver telemetry.
     * @return Operation status, true in case of success, false otherwise.
     *
     * The contents of the telemetry object is undefined in case of the failure.
     */
    bool GetReceiverTelemetry(ReceiverTelemetry& telemetry);

    /**
     * @brief Queries the comm driver for the transmitter telemetry.
     *
     * @param[out] telemetry Reference to object that should be filled with current transmitter telemetry.
     * @return Operation status, true in case of success, false otherwise.
     *
     * The contents of the telemetry object is undefined in case of the failure.
     */
    bool GetTransmitterTelemetry(TransmitterTelemetry& telemetry);

    /**
     * @brief Adds the requested frame to the send queue.
     *
     * @param[in] frame Buffer containing frame contents.
     * @return Operation status, true in case of success, false otherwise.
     */
    bool SendFrame(gsl::span<const std::uint8_t> frame);

    /**
     * @brief Requests the contents of the oldest received frame from the queue.
     *
     * @param[in] buffer Memory buffer that should be used for frame data retrieval.
     * @param[out] frame Reference to object that should be filled with the data describing the oldest
     * available received frame.
     * @return Operation status, true in case of success, false otherwise.
     *
     * The contents of the frame object is undefined in case of the failure.
     * If the buffer too short to fit entire frame and it header only the part of the frame that will
     * fit into this frame will be retrieved, parsed and returned back to the called.
     */
    bool ReceiveFrame(gsl::span<std::uint8_t> buffer, Frame& frame);

    /**
     * @brief This procedure sets the beacon frame for the passed comm object.
     *
     * @param[in] beaconData Reference to object describing new beacon.
     * See the definition of the CommBeacon for details.
     * @return Operation status, true in case of success, false otherwise.
     */
    bool SetBeacon(const Beacon& beaconData);

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
    bool SetTransmitterStateWhenIdle(IdleState requestedState);

    /**
     * @brief Set the transmitter baud rate.
     *
     * @param[in] bitrate New transmitter baud rate.
     * @return Operation status, true in case of success, false otherwise.
     */
    bool SetTransmitterBitRate(Bitrate bitrate);

    /**
     * @brief Queries the comm driver object for current transmitter state.
     *
     * @param[out] state Reference to object that should be filled with the data describing
     * the current transmitter state.
     * @return Operation status, true in case of success, false otherwise.
     *
     * The contents of the state object is undefined in case of the failure.
     */
    bool GetTransmitterState(TransmitterState& state);

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

    /**
     * @brief Resets the internal hardware watchdog.
     *
     * @return Operation status, true in case of success, false otherwise.
     */
    bool ResetWatchdogReceiver();

    /**
     * @brief Resets the internal hardware watchdog.
     *
     * @return Operation status, true in case of success, false otherwise.
     */
    bool ResetWatchdogTransmitter();

  private:
    /** @brief Comm driver lower interface. */
    drivers::i2c::II2CBus& _low;

    /** @brief Comm driver upper interface. */
    IHandleFrame& _frameHandler;

    /** @brief Handle to comm background task. */
    void* _pollingTaskHandle;

    /** @brief Handle to event group used to communicate with background task. */
    OSEventGroupHandle _pollingTaskFlags;

    bool SendCommand(Address address, std::uint8_t command);
    bool SendCommandWithResponse(Address address, std::uint8_t command, gsl::span<std::uint8_t> outBuffer);
    void PollHardware();
    [[noreturn]] static void CommTask(void* param);
};

COMM_END

#endif
