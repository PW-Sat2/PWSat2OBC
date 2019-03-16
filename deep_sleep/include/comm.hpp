#ifndef DEEP_SLEEP_COMM_DEEP_COMM_DRIVER_HPP
#define DEEP_SLEEP_COMM_DEEP_COMM_DRIVER_HPP

#pragma once

#include "comm/comm.hpp"
#include "gsl/span"
#include "standalone/i2c/i2c.hpp"

enum class StandaloneFrameType
{
    None = 0,
    SendBeacon = 1,
    Reboot = 2
};

class StandaloneFrame
{
  public:
    /**
     * @brief ctor.
     */
    StandaloneFrame();

    /**
     * @brief ctor.
     * @param[in] fullSize Full frame size.
     * @param[in] data span that contains entire received frame
     */
    StandaloneFrame(std::uint16_t fullSize, gsl::span<std::uint8_t> data);

    /**
     * @brief Returns span that contains entire received frame.
     * @return Frame contents span
     */
    const gsl::span<std::uint8_t>& Payload() const;

    /**
     * @brief Returns Current frame size.
     * @return Frame size in bytes.
     */
    std::uint16_t Size() const;

    /**
     * @brief Returns Actual frame size.
     * @return Frame size in bytes.
     */
    std::uint16_t FullSize() const;

    /**
     * @brief Verifies size of this frame.
     * @return Verification status.
     * @retval true Frame size is valid.
     * @retval false Frame size is invalid.
     */
    bool IsSizeValid() const;

    /**
     * @brief Verifies whether the entire frame content is represented by this object.
     * @return Verification status.
     * @retval true Entire frame is available.
     * @retval false At least one payload byte is missing.
     */
    bool IsComplete() const;

    /**
     * @brief Verifies state of this frame object.
     * @return Verification status.
     * @retval true There are no errors and entire frame payload is available.
     * @retval false There are errors or there is at least one byte missing.
     */
    bool Verify() const;

  private:
    /** @brief Complete size of the current frame. */
    std::uint16_t fullFrameSize;

    /** @brief Frame content. */
    gsl::span<std::uint8_t> content;
};

inline const gsl::span<uint8_t>& StandaloneFrame::Payload() const
{
    return this->content;
}

inline std::uint16_t StandaloneFrame::Size() const
{
    return gsl::narrow_cast<std::uint16_t>(this->content.size());
}

inline std::uint16_t StandaloneFrame::FullSize() const
{
    return this->fullFrameSize;
}

inline bool StandaloneFrame::IsSizeValid() const
{
    return this->fullFrameSize <= COMM::MaxDownlinkFrameSize;
}

inline bool StandaloneFrame::IsComplete() const
{
    return this->fullFrameSize == gsl::narrow_cast<std::uint16_t>(this->content.size());
}

inline bool StandaloneFrame::Verify() const
{
    return IsSizeValid() && IsComplete();
}

/**
 * @brief This type describe comm driver global state.
 * @ingroup LowerCommDriver
 * @remark Do not access directly the fields of this type, instead use the comm driver interface to
 * perform requested action.
 */

class StandaloneComm
{
  public:
    /**
     * Constructs new instance of COMM low-level driver
     * @param[in] i2c I2C bus used to communicate with device
     * @param[in] sleepDriver Driver for resetting OBC
     */
    StandaloneComm(StandaloneI2C& i2c);

    /**
     * @brief Queries comm driver for a number of received and not yet processed frames.
     *
     * @return Operation status. See the definition of the CommReceiverFrameCount for details.
     */
    devices::comm::ReceiverFrameCount GetFrameCount();

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
     * @returns True if any frame was received
     *
     * This function queries the state of the underlying hardware and processes any not yet received frames.
     * Additionally it resets hardware watchdog either via transmitter or via receiver.
     */
    StandaloneFrameType PollHardware();

    /**
     * @brief Set the transmitter baud rate.
     *
    0 * @param[in] bitrate New transmitter baud rate.
     * @return Operation status, true in case of success, false otherwise.
     */
    bool SetTransmitterBitRate(COMM::Bitrate bitrate);

    /**
     * @brief Adds the requested frame to the send queue.
     *
     * @param[in] frame Buffer containing frame contents.
     * @return Operation status, true in case of success, false otherwise.
     */
    bool SendFrame(gsl::span<const std::uint8_t> frame);

  private:
    /**
     * @brief Sends passed no argument command to the device with requested address.
     * @param[in] address Address of the device which should receive the command.
     * @param[in] command Command code to send.
     * @return Operation status, true in case of success, false otherwise.
     */
    bool SendCommand(devices::comm::Address address, std::uint8_t command);

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
    bool SendCommandWithResponse(devices::comm::Address address, //
        std::uint8_t command,                                    //
        gsl::span<std::uint8_t> outBuffer);

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
    bool SendBufferWithResponse(devices::comm::Address address, //
        gsl::span<const std::uint8_t> inputBuffer,              //
        gsl::span<uint8_t> outBuffer);

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
    bool GetFrame(gsl::span<std::uint8_t> buffer, int retryCount, StandaloneFrame& frame);

    /**
     * @brief This procedure is responsible for downloading single frame from the hardware
     * and pushing it through the frame the processing pipeline.
     *
     * Once the frame is processed this function will try to remove it from the hardware.
     */
    StandaloneFrameType ProcessSingleFrame();
    bool ReceiveFrameInternal(gsl::span<std::uint8_t> buffer, StandaloneFrame& frame);
    bool RemoveFrameInternal();
    StandaloneFrameType HandleFrame(StandaloneFrame& frame);

    /** @brief Comm driver lower interface. */
    StandaloneI2C& _i2c;
};

#endif /* DEEP_SLEEP_COMM_DEEP_COMM_DRIVER_HPP */
