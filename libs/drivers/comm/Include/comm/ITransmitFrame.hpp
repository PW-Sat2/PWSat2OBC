#ifndef LIBS_DRIVERS_COMM_ITRANSMIT_FRAME_HPP
#define LIBS_DRIVERS_COMM_ITRANSMIT_FRAME_HPP

#pragma once

#include <cstdint>
#include "comm.hpp"
#include "gsl/span"

COMM_BEGIN

/**
 * @brief Frame transmitter interface.
 * @ingroup LowerCommDriver
 */
struct ITransmitFrame
{
    /**
     * @brief Adds the requested frame to the send queue.
     *
     * @param[in] frame Buffer containing frame contents.
     * @return Operation status, true in case of success, false otherwise.
     */
    virtual bool SendFrame(gsl::span<const std::uint8_t> frame) = 0;
};

COMM_END

#endif
