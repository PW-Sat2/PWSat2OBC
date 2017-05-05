#ifndef LIBS_DRIVERS_COMM_COMM_DRIVER_HPP
#define LIBS_DRIVERS_COMM_COMM_DRIVER_HPP

#pragma once

#include "IBeaconController.hpp"
#include "ITransmitter.hpp"
#include "base/os.h"
#include "comm.hpp"
#include "error_counter/error_counter.hpp"
#include "gsl/span"
#include "i2c/forward.h"

COMM_BEGIN

/**
 * @brief This type describe comm driver global state.
 * @ingroup LowerCommDriver
 * @remark Do not access directly the fields of this type, instead use the comm driver interface to
 * perform requested action.
 */
class CommObject final : public ITransmitter, public IBeaconController
{
  public:
    /**
     * Constructs new instance of COMM low-level driver
     * @param[in] errors Error counting mechanism
     * @param[in] low I2C bus used to communicate with device
     * @param[in] upperInterface Reference to object responsible for interpreting received frames
     */
    CommObject(error_counter::ErrorCounting& errors, drivers::i2c::II2CBus& low, IHandleFrame& upperInterface);

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
     * This procedure will ensure that setting beacon will not erase any frames from
     * transmitter's output buffer by first sending beacon as a regular frame and setting
     * the beacon itself only if there are no frames that are waiting to te send by the transmitter.
     * @param[in] beacon Reference to object describing new beacon.
     * See the definition of the CommBeacon for details.
     * @return Operation status, true in case of success, false otherwise, None in case there were frames in
     * transmitter queue.
     */
    virtual Option<bool> SetBeacon(const Beacon& beacon) override final;

    /**
     * @brief Clears any beacon that is currently set in the transceiver. If a beacon transmission
     * is currently in progress, this transmission will be completed.
     *
     * @return Operation status, true in case of success, false otherwise.
     */
    virtual bool ClearBeacon() override final;

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

    /**
     * @brief This procedure queries current hardware for state changes.
     *
     * This function queries the state of the underlying hardware and processes any not yet received frames.
     * Additionally it resets hardware watchdog either via transmitter or via receiver.
     */
    void PollHardware();

    /** @brief Error counter type */
    using ErrorCounter = error_counter::ErrorCounter<1>;

  private:
    /**
     * @brief Sends passed no argument command to the device with requested address.
     * @param[in] address Address of the device which should receive the command.
     * @param[in] command Command code to send.
     * @return Operation status, true in case of success, false otherwise.
     */
    bool SendCommand(Address address, std::uint8_t command);

    /**
     * @brief Sends passed no argument command to the device with requested address.
     *
     * This method expected to receive response from the device that should be written to the passed buffer.
     * This method will try to read at most the number of bytes that is equal to the passed buffer size, any
     * additional bytes that are send by the device that will not fit into the passed buffer will be discarded.
     * @param[in] address Address of the device which should receive the command.
     * @param[in] command Command code to send.
     * @param[out] outBuffer Buffer for the device's response.
     * @return Operation status, true in case of success, false otherwise.
     */
    bool SendCommandWithResponse(Address address, std::uint8_t command, gsl::span<std::uint8_t> outBuffer);

    /**
     * @brief Sends contents of passed data buffer to the device with requested address.
     *
     * This method expected to receive response from the device that should be written to the passed buffer.
     * This method will try to read at most the number of bytes that is equal to the passed buffer size, any
     * additional bytes that are send by the device that will not fit into the passed buffer will be discarded.
     * This method uses two i2c transactions one for writing data, one for reading the data that are separated
     * by 2ms delay in order to give the hardware time to prepare the response in case it does not support
     * i2c clock stretching for signaling that it is not ready with the answer.
     * @param[in] address Address of the device which should receive the command.
     * @param[in] inputBuffer Buffer with the data frame that should be send to the hardware.
     * @param[out] outBuffer Buffer for the device's response.
     * @return Operation status, true in case of success, false otherwise.
     */
    bool SendBufferWithResponse(Address address,   //
        gsl::span<const std::uint8_t> inputBuffer, //
        gsl::span<uint8_t> outBuffer               //
        );
    /**
     * @brief This procedure will try to download the oldest not yet processed frame from the hardware.
     *
     * The passed buffer is used as frame content storage area and should be large enough to fit
     * the entire frame (with its header). IF the buffer is not long enough to contain the entire frame with
     * its header then the part of the frame that will not fit into the buffer  will be discarded.
     *
     * @param[in] buffer Buffer for the frame contents. This method will try to read at most the number of bytes
     * that is equal to the passed buffer size, any additional bytes that are send by the device
     * that will not fit into the passed buffer will be discarded.
     * @param[in] retryCount Number of frame download & processing retrials. Passing value less than 1
     * will make this function as no operation.
     * @param[out] frame Frame object that will be filled with the received frame details. This variable is
     * valid only in case of success and is left untouched in case of failure.
     * @return Operation status, true in case of success, false otherwise.
     */
    bool GetFrame(gsl::span<std::uint8_t> buffer, int retryCount, Frame& frame);

    /**
     * @brief This procedure is responsible for downloading single frame from the hardware
     * and pushing it through the frame the processing pipeline.
     *
     * Once the frame is processed this function will try to remove it from the hardware.
     */
    void ProcessSingleFrame();

    /**
     * @brief This procedure sets the beacon frame for the passed comm object.
     *
     * @param[in] beacon Reference to object describing new beacon.
     * See the definition of the CommBeacon for details.
     * @return Operation status, true in case of success, false otherwise.
     */
    bool UpdateBeacon(const Beacon& beacon);

    /**
     * @brief Adds the requested frame to the send queue.
     *
     * @param[in] frame Buffer containing frame contents.
     * @param[out] remainingSlots Number of free slots in transmitter's output buffer.
     * @return Operation status, true in case of success, false otherwise.
     */
    bool ScheduleFrameTransmission(gsl::span<const std::uint8_t> frame, std::uint8_t& remainingSlots);

    /**
     * @brief Internal communication module task entry point.
     * @param[in] param Task execution context. This should be pointer to the task owner object.
     */
    [[noreturn]] static void CommTask(void* param);

    /** @brief Error counter */
    ErrorCounter _error;

    /** @brief Comm driver lower interface. */
    drivers::i2c::II2CBus& _low;

    /** @brief Comm driver upper interface. */
    IHandleFrame& _frameHandler;

    /** @brief Handle to comm background task. */
    void* _pollingTaskHandle;

    /** @brief Event group used to communicate with background task. */
    EventGroup _pollingTaskFlags;
};

inline bool CommObject::SendFrame(gsl::span<const std::uint8_t> frame)
{
    std::uint8_t remainingBufferSize;
    return ScheduleFrameTransmission(frame, remainingBufferSize);
}

COMM_END

#endif
