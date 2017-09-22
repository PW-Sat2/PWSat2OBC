#ifndef LIBS_DRIVERS_COMM_TELEMETRY_HPP
#define LIBS_DRIVERS_COMM_TELEMETRY_HPP

#pragma once

#include <cstdint>
#include "comm.hpp"
#include "utils.h"

COMM_BEGIN

/**
 * @brief This class represents complete communication module telemetry object as it is presented and exposed
 * to the rest of the system.
 * @telemetry_element
 */
class CommTelemetry
{
  public:
    /**
     * @brief Comm telemetry unique identifier.
     */
    static constexpr int Id = 12;

    /**
     * @brief ctor.
     */
    CommTelemetry();

    /**
     * @brief ctor.
     * @param[in] receiver Current receiver telemetry
     * @param[in] transmitter Current transmitter telemetry
     */
    CommTelemetry(const TransmitterTelemetry& transmitter, const ReceiverTelemetry& receiver);

    /**
     * @brief Write the comm telemetry to passed buffer writer object.
     * @param[in] writer Buffer writer object that should be used to write the serialized state.
     */
    void Write(BitWriter& writer) const;

    /**
     * @brief Returns size of the serialized state in bits.
     * @return Size of the serialized state in bits.
     */
    static constexpr std::uint32_t BitSize();

  private:
    TransmitterTelemetry _transmitter;
    ReceiverTelemetry _receiver;
};

constexpr std::uint32_t CommTelemetry::BitSize()
{
    return TransmitterTelemetry::BitSize() + ReceiverTelemetry::BitSize();
}

static_assert(CommTelemetry::BitSize() == 206, "Incorrect telemetry format");

COMM_END

#endif
