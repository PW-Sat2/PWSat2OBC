#ifndef LIBS_DRIVERS_COMM_TELEMETRY_HPP
#define LIBS_DRIVERS_COMM_TELEMETRY_HPP

#pragma once

#include <cstdint>
#include "comm.hpp"

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
     * @brief Read the comm telemetry from passed reader.
     * @param[in] reader Buffer reader that should be used to read the serialized state.
     */
    void Read(Reader& reader);

    /**
     * @brief Write the comm telemetry to passed buffer writer object.
     * @param[in] writer Buffer writer object that should be used to write the serialized state
     * of the time subsystem state.
     */
    void Write(Writer& writer) const;

    /**
     * @brief Returns size of the serialized state in bytes.
     * @return Size of the serialized state in bytes.
     */
    static constexpr std::uint32_t Size();

    /**
     * @brief Procedure that verifies whether two CommTelemetry objects differ enough to so they should be
     * treated as two distinct states.
     * @param[in] arg Object to compare to.
     *
     * @remark This version is based on value equality.
     * @return True in case objects are different, false otherwise.
     */
    bool IsDifferent(const CommTelemetry& arg) const;

  private:
    /** @brief Raw measurement value of the transmitter current consumption. */
    std::uint16_t transmitterCurrentConsumption;

    /** @brief Raw measurement value of the receiver current consumption. */
    std::uint16_t receiverCurrentConsumption;

    /** @brief Raw measurement value of the instantaneous Doppler offset of the signal at the receiver port. */
    std::uint16_t dopplerOffset;

    /** @brief Raw measurement value of the power bus voltage.*/
    std::uint16_t vcc;

    /** @brief Raw measurement value of the local oscillator temperature. */
    std::uint16_t oscilatorTemperature;

    /** @brief Raw measurement value of the power amplifier temperature. */
    std::uint16_t receiverAmplifierTemperature;

    /** @brief Raw measurement value of the instantaneous signal strength of the signal at the receiver. */
    std::uint16_t signalStrength;

    /** @brief Raw measurement value of the instantaneous RF reflected power at the transmitter port. */
    std::uint16_t rFReflectedPower;

    /** @brief Raw measurement value of the power amplifier temperature. */
    std::uint16_t transmitterAmplifierTemperature;

    /** @brief Raw measurement value of the instantaneous RF forward power at the transmitter port. */
    std::uint16_t rFForwardPower;

    TransmitterState transmitterState;
};

constexpr std::uint32_t CommTelemetry::Size()
{
    return 10 * sizeof(std::uint16_t) + 3 * sizeof(std::uint8_t);
}

COMM_END

#endif
