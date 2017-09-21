#ifndef LIBS_DRIVERS_COMM_ITRANSMITTER_HPP
#define LIBS_DRIVERS_COMM_ITRANSMITTER_HPP

#include <cstdint>
#include "comm.hpp"
#include "gsl/span"

COMM_BEGIN

/**
 * @brief Transmitter interface.
 * @ingroup LowerCommDriver
 */
struct ITransmitter
{
    /**
     * @brief Adds the requested frame to the send queue.
     *
     * @param[in] frame Buffer containing frame contents.
     * @return Operation status, true in case of success, false otherwise.
     */
    virtual bool SendFrame(gsl::span<const std::uint8_t> frame) = 0;

    /**
     * @brief Queries the comm driver for the transmitter telemetry.
     *
     * @param[out] telemetry Reference to object that should be filled with current transmitter telemetry.
     * @return Operation status, true in case of success, false otherwise.
     *
     * The contents of the telemetry object is undefined in case of the failure.
     */
    virtual bool GetTransmitterTelemetry(TransmitterTelemetry& telemetry) = 0;

    /**
     * @brief Set the transmitter state when there are no more frames to sent.
     *
     * @param[in] requestedState New transmitter state when it is idle.
     * @return Operation status, true in case of success, false otherwise.
     */
    virtual bool SetTransmitterStateWhenIdle(IdleState requestedState) = 0;

    /**
     * @brief Set the transmitter baud rate.
     *
     * @param[in] bitrate New transmitter baud rate.
     * @return Operation status, true in case of success, false otherwise.
     */
    virtual bool SetTransmitterBitRate(Bitrate bitrate) = 0;

    /**
     * @brief Resets the transmitter hardware associated with the requested comm object.
     *
     * @return Operation status, true in case of success, false otherwise.
     */
    virtual bool ResetTransmitter() = 0;
};

COMM_END

#endif /* LIBS_DRIVERS_COMM_ITRANSMITTER_HPP */
