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
    static constexpr int Id = 3;

    /**
     * @brief ctor.
     */
    CommTelemetry();

    /**
     * @brief ctor.
     * @param[in] receiver Current receiver telemetry
     * @param[in] transmitter Current transmitter telemetry
     * @param[in] state Current transmitter configuration
     * @param[in] uptime Current transmitter uptime
     */
    CommTelemetry(const ReceiverTelemetry& receiver, //
        const TransmitterTelemetry& transmitter,
        const TransmitterState& state,
        const Uptime& uptime);

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

    /**
     * @brief Returns raw measurement value of the transmitter current consumption.
     * @return Raw measurement value of the transmitter current consumption.
     */
    uint12_t TransmitterCurrentConsumption() const noexcept;

    /**
     * @brief Returns raw measurement value of the receiver current consumption.
     * @return Raw measurement value of the receiver current consumption.
     */
    uint12_t ReceiverCurrentConsumption() const noexcept;

    /**
     * @brief Returns Raw measurement value of the instantaneous Doppler offset.
     * @return Raw measurement value of the instantaneous Doppler offset of the signal at the receiver port.
     */
    uint12_t DopplerOffset() const noexcept;

    /**
     * @brief Returns raw measurement value of the power bus voltage.
     * @return Raw measurement value of the power bus voltage.
     */
    uint12_t Vcc() const noexcept;

    /**
     * @brief Returns Raw measurement value of the local oscillator temperature.
     * @return Raw measurement value of the local oscillator temperature.
     */
    uint12_t OscilatorTemperature() const noexcept;

    /**
     * @brief Returns raw measurement value of the power amplifier temperature.
     * @return Raw measurement value of the power amplifier temperature.
     */
    uint12_t ReceiverAmplifierTemperature() const noexcept;

    /**
     * @brief Returns raw measurement value of the instantaneous signal strength of the signal at the receiver.
     * @return Raw measurement value of the instantaneous signal strength of the signal at the receiver.
     */
    uint12_t SignalStrength() const noexcept;

    /**
     * @brief Returns raw measurement value of the instantaneous RF reflected power at the transmitter port.
     * @return Raw measurement value of the instantaneous RF reflected power at the transmitter port.
     */
    uint12_t RFReflectedPower() const noexcept;

    /**
     * @brief Returns raw measurement value of the power amplifier temperature.
     * @return Raw measurement value of the power amplifier temperature.
     */
    uint12_t TransmitterAmplifierTemperature() const noexcept;

    /**
     * @brief Returns raw measurement value of the instantaneous RF forward power at the transmitter port.
     * @return Raw measurement value of the instantaneous RF forward power at the transmitter port.
     */
    uint12_t RFForwardPower() const noexcept;

    /**
     * @brief Returns object containing transmitter mcu uptime.
     * @return Transmitter mcu uptime.
     */
    const Uptime& TransmitterUptime() const noexcept;

    /**
     * @brief Returns information whether transmitter carrier is currently being send.
     * @return True in case transmitter carrier is currently being send, false otherwise.
     */
    bool TransmitterIdleState() const noexcept;

    /**
     * @brief Returns information whether beacon is active.
     * @return True in case beacon is currently active, false otherwise.
     */
    bool BeaconState() const noexcept;

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

    Uptime transmitterUptime;

    bool transmitterIdleState;

    bool beaconState;
};

constexpr std::uint32_t CommTelemetry::BitSize()
{
    return 10 * uint12_t::Size +          //
        decltype(Uptime::seconds)::Size + //
        decltype(Uptime::minutes)::Size + //
        decltype(Uptime::hours)::Size +   //
        decltype(Uptime::days)::Size +    //
        2;
}

inline uint12_t CommTelemetry::TransmitterCurrentConsumption() const noexcept
{
    return this->transmitterCurrentConsumption;
}

inline uint12_t CommTelemetry::ReceiverCurrentConsumption() const noexcept
{
    return this->receiverCurrentConsumption;
}

inline uint12_t CommTelemetry::DopplerOffset() const noexcept
{
    return this->dopplerOffset;
}

inline uint12_t CommTelemetry::Vcc() const noexcept
{
    return this->vcc;
}

inline uint12_t CommTelemetry::OscilatorTemperature() const noexcept
{
    return this->oscilatorTemperature;
}

inline uint12_t CommTelemetry::ReceiverAmplifierTemperature() const noexcept
{
    return this->receiverAmplifierTemperature;
}

inline uint12_t CommTelemetry::SignalStrength() const noexcept
{
    return this->signalStrength;
}

inline uint12_t CommTelemetry::RFReflectedPower() const noexcept
{
    return this->rFReflectedPower;
}

inline uint12_t CommTelemetry::TransmitterAmplifierTemperature() const noexcept
{
    return this->transmitterAmplifierTemperature;
}

inline uint12_t CommTelemetry::RFForwardPower() const noexcept
{
    return this->rFForwardPower;
}

inline const Uptime& CommTelemetry::TransmitterUptime() const noexcept
{
    return this->transmitterUptime;
}

inline bool CommTelemetry::TransmitterIdleState() const noexcept
{
    return this->transmitterIdleState;
}

inline bool CommTelemetry::BeaconState() const noexcept
{
    return this->beaconState;
}

COMM_END

#endif
