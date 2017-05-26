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
 */
class CommTelemetry
{
  public:
    /**
     * @brief TimeState telemetry unique identifier.
     */
    static constexpr int Id = 2;

    /**
     * @brief ctor.
     */
    CommTelemetry();

    /**
     * @brief ctor.
     * @param[in] receiver Current receiver telemetry
     * @param[in] transmitter Current transmitter telemetry
     * @param[in] state Current transmitter configuration
     */
    CommTelemetry(const ReceiverTelemetry& receiver, const TransmitterTelemetry& transmitter, const TransmitterState& state);

    /**
     * @brief Write the comm telemetry to passed buffer writer object.
     * @param[in] writer Buffer writer object that should be used to write the serialized state
     * of the time subsystem state.
     */
    void Write(BitWriter& writer) const;

    /**
     * @brief Returns size of the serialized state in bytes.
     * @return Size of the serialized state in bytes.
     */
    static constexpr std::uint32_t BitSize();

  private:
    /** @brief Raw measurement value of the transmitter current consumption. */
    uint12_t transmitterCurrentConsumption;

    /** @brief Raw measurement value of the receiver current consumption. */
    uint12_t receiverCurrentConsumption;

    /** @brief Raw measurement value of the instantaneous Doppler offset of the signal at the receiver port. */
    uint12_t dopplerOffset;

    /** @brief Raw measurement value of the power bus voltage.*/
    uint12_t vcc;

    /** @brief Raw measurement value of the local oscillator temperature. */
    uint12_t oscilatorTemperature;

    /** @brief Raw measurement value of the power amplifier temperature. */
    uint12_t receiverAmplifierTemperature;

    /** @brief Raw measurement value of the instantaneous signal strength of the signal at the receiver. */
    uint12_t signalStrength;

    /** @brief Raw measurement value of the instantaneous RF reflected power at the transmitter port. */
    uint12_t rFReflectedPower;

    /** @brief Raw measurement value of the power amplifier temperature. */
    uint12_t transmitterAmplifierTemperature;

    /** @brief Raw measurement value of the instantaneous RF forward power at the transmitter port. */
    uint12_t rFForwardPower;

    BitValue<std::uint8_t, 6> transmitterUptimeSeconds;

    BitValue<std::uint8_t, 5> transmitterUptimeMinutes;

    BitValue<std::uint8_t, 5> transmitterUptimeHours;

    std::uint8_t transmitterUptimeDays;

    bool transmitterIdleState;

    bool beaconState;
};

constexpr std::uint32_t CommTelemetry::BitSize()
{
    return 10 * uint12_t::Size +                                  //
        decltype(CommTelemetry::transmitterUptimeSeconds)::Size + //
        decltype(CommTelemetry::transmitterUptimeMinutes)::Size + //
        decltype(CommTelemetry::transmitterUptimeHours)::Size +   //
        decltype(CommTelemetry::transmitterUptimeSeconds)::Size + //
        8 + 2;
}

COMM_END

#endif
