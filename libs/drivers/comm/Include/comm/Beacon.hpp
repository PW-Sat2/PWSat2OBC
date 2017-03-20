#ifndef LIBS_DRIVERS_COMM_BEACON_HPP
#define LIBS_DRIVERS_COMM_BEACON_HPP

#pragma once

#include <chrono>
#include "comm.hpp"

COMM_BEGIN

/**
 * @brief This type describes the comm beacon.
 * @ingroup LowerCommDriver
 */
class Beacon
{
  public:
    /**
     * @brief ctor.
     */
    Beacon();

    /**
     * @brief ctor.
     * @param[in] beaconPeriod Requested time in seconds between two subsequent beacon transmissions.
     * This is the interval between the starts of two transmissions of the beacon. This interval
     * has a minimum value of 0 and a maximum value of 3000. If an interval of more than 3000 seconds
     * is specified, the interval will automatically be set to 3000.
     * @param[in] contents Requested Beacon content.
     */

    Beacon(std::chrono::seconds beaconPeriod, gsl::span<const std::uint8_t> contents);

    /**
     * @brief Returns repeat interval of the beacon in seconds.
     * @return Time in seconds between two subsequent beacon transmissions.
     */
    std::chrono::seconds Period() const;

    /**
     * @brief Returns beacon frame content.
     * @return Beacon frame content.
     */
    gsl::span<const std::uint8_t> Contents() const;

  private:
    /**
     * @brief Repeat interval of the beacon in seconds.
     */
    std::chrono::seconds period;

    /** @brief Beacon frame contents. */
    gsl::span<const std::uint8_t> payload;
};

inline std::chrono::seconds Beacon::Period() const
{
    return this->period;
}

inline gsl::span<const std::uint8_t> Beacon::Contents() const
{
    return this->payload;
}

COMM_END

#endif
