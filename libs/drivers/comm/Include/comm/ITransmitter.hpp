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
     * @brief Set the transmitter state when there are no more frames to sent.
     *
     * @param[in] requestedState New transmitter state when it is idle.
     * @return Operation status, true in case of success, false otherwise.
     */
    virtual bool SetTransmitterStateWhenIdle(IdleState requestedState) = 0;

    /**
     * @brief Adds the requested frame to the send queue.
     *
     * @param[in] frame Buffer containing frame contents.
     * @return Operation status, true in case of success, false otherwise.
     */
    virtual bool SendFrame(gsl::span<const std::uint8_t> frame) = 0;
};

COMM_END

#endif /* LIBS_DRIVERS_COMM_ITRANSMITTER_HPP */
