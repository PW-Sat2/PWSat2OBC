#ifndef DEEP_SLEEP_COMM_DEEP_COMM_DRIVER_HPP
#define DEEP_SLEEP_COMM_DEEP_COMM_DRIVER_HPP

#pragma once

#include "comm/comm.hpp"
#include "gsl/span"
#include "standalone/i2c/i2c.hpp"

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
    bool PollHardware();

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
    bool SendCommandWithResponse(devices::comm::Address address,               //
        std::uint8_t command,                                   //
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
    bool SendBufferWithResponse(devices::comm::Address address,                //
        gsl::span<const std::uint8_t> inputBuffer,              //
        gsl::span<uint8_t> outBuffer);

    /** @brief Comm driver lower interface. */
    StandaloneI2C& _i2c;
};

#endif /* DEEP_SLEEP_COMM_DEEP_COMM_DRIVER_HPP */