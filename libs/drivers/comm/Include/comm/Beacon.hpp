#ifndef LIBS_DRIVERS_COMM_BEACON_HPP
#define LIBS_DRIVERS_COMM_BEACON_HPP

#pragma once

#include "comm.hpp"

COMM_BEGIN

/**
 * @brief This type describes the comm beacon.
 */
class Beacon
{
  public:
    Beacon();

    Beacon(std::uint16_t, gsl::span<const std::uint8_t> contents);

    std::uint16_t Period() const;

    gsl::span<const std::uint8_t> Contents() const;

  private:
    /**
     * @brief Repeat interval of the beacon in seconds.
     *
     * This is the interval between the starts of two transmissions of the beacon. This interval
     * has a minimum value of 0 and a maximum value of 3000. If an interval of more than 3000 seconds
     * is specified, the interval will automatically be set to 3000.
     */
    std::uint16_t period;

    /** @brief Beacon frame contents. */
    gsl::span<const std::uint8_t> payload;
};

inline std::uint16_t Beacon::Period() const
{
    return this->period;
}

inline gsl::span<const std::uint8_t> Beacon::Contents() const
{
    return this->payload;
}

COMM_END

#endif
