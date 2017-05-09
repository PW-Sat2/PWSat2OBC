#ifndef LIBS_DRIVERS_COMM_IHANDLE_FRAME_HPP
#define LIBS_DRIVERS_COMM_IHANDLE_FRAME_HPP

#pragma once

#include "comm.hpp"

COMM_BEGIN

/**
 * @brief Comm driver upper interface.
 * @ingroup LowerCommDriver
 */
struct IHandleFrame
{
    /**
     * @brief Method called on each incoming frame.
     * @param[in] transmitter Reference to object that can be used to send frame back
     * @param[in] frame Reference to structure describing received frame
     *
     */
    virtual void HandleFrame(ITransmitter& transmitter, Frame& frame) = 0;
};

COMM_END

#endif
