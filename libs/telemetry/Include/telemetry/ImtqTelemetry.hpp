#ifndef LIBS_TELEMETRY_IMTQ_HPP
#define LIBS_TELEMETRY_IMTQ_HPP

#pragma once

#include <array>
#include "base/fwd.hpp"
#include "fwd.hpp"
#include "imtq/fwd.hpp"
#include "telemetry/BasicTelemetry.hpp"
#include "utils.h"

namespace telemetry
{
    /**
     * @brief This type represents current imtq house keeping telemetry
     * @telemetry_element
     */
    class ImtqHousekeeping
    {
      public:
        /**
         * @brief ctor.
         */
        ImtqHousekeeping();

        /**
         * @brief ctor.
         * @param digitalVoltage Current digital voltage.
         * @param analogVoltage Current analog voltage.
         * @param digitalCurrent Current digital current.
         * @param analogCurrent Current analog current.
         * @param mcuTemperature Current mcu temperature.
         */
        ImtqHousekeeping(devices::imtq::VoltageInMiliVolt digitalVoltage,
            devices::imtq::VoltageInMiliVolt analogVoltage,
            devices::imtq::Current digitalCurrent,
            devices::imtq::Current analogCurrent,
            devices::imtq::TemperatureMeasurement mcuTemperature);

        /**
         * @brief Write the imtq house keeping element to passed buffer writer object.
         * @param[in] writer Buffer writer object that should be used to write the serialized state.
         */
        void Write(BitWriter& writer) const;

        /**
         * @brief Returns size of the serialized state in bits.
         * @return Size of the serialized state in bits.
         */
        static constexpr std::uint32_t BitSize();

        /**
         * @brief Property accessor;
         * @return Current value of digital voltage supply.
         */
        devices::imtq::VoltageInMiliVolt DigitalVoltage() const noexcept;

        /**
         * @brief Property accessor;
         * @return Current value of analog voltage supply.
         */
        devices::imtq::VoltageInMiliVolt AnalogVoltage() const noexcept;

        /**
         * @brief Property accessor;
         * @return Current value of digital current supply.
         */
        devices::imtq::Current DigitalCurrent() const noexcept;

        /**
         * @brief Property accessor;
         * @return Current value of analog current supply.
         */
        devices::imtq::Current AnalogCurrent() const noexcept;

        /**
         * @brief Property accessor;
         * @return Current value mcu temperature.
         */
        devices::imtq::TemperatureMeasurement McuTemperature() const noexcept;

      private:
        devices::imtq::VoltageInMiliVolt digitalVoltage;
        devices::imtq::VoltageInMiliVolt analogVoltage;
        devices::imtq::Current digitalCurrent;
        devices::imtq::Current analogCurrent;
        devices::imtq::TemperatureMeasurement mcuTemperature;
    };

    inline devices::imtq::VoltageInMiliVolt ImtqHousekeeping::DigitalVoltage() const noexcept
    {
        return this->digitalVoltage;
    }

    inline devices::imtq::VoltageInMiliVolt ImtqHousekeeping::AnalogVoltage() const noexcept
    {
        return this->analogVoltage;
    }

    inline devices::imtq::Current ImtqHousekeeping::DigitalCurrent() const noexcept
    {
        return this->digitalCurrent;
    }

    inline devices::imtq::Current ImtqHousekeeping::AnalogCurrent() const noexcept
    {
        return this->analogCurrent;
    }

    inline devices::imtq::TemperatureMeasurement ImtqHousekeeping::McuTemperature() const noexcept
    {
        return this->mcuTemperature;
    }

    constexpr std::uint32_t ImtqHousekeeping::BitSize()
    {
        return Aggregate<decltype(digitalVoltage),
            decltype(analogVoltage),
            decltype(digitalCurrent),
            decltype(analogCurrent),
            decltype(mcuTemperature)>;
    }

    static_assert(ImtqHousekeeping::BitSize() == 80, "Invalid serialized size");

    /**
     * @brief This type represents part of telemetry that contains most recent imtq state.
     * @telemetry_element
     */
    class ImtqState
    {
      public:
        /**
         * @brief ctor.
         */
        ImtqState();

        /**
         * @brief ctor.
         * @param mode Current imtq mode
         * @param errorCode Last imtq error code
         * @param configurationUpdated Flag indicating whether imtq state has been recently updated
         * @param uptime Current imtq uptime
         */
        ImtqState(devices::imtq::Mode mode, std::uint8_t errorCode, bool configurationUpdated, std::chrono::seconds uptime);

        /**
         * @brief Write the imtq state element to passed buffer writer object.
         * @param[in] writer Buffer writer object that should be used to write the serialized state.
         */
        void Write(BitWriter& writer) const;

        /**
         * @brief Returns size of the serialized state in bits.
         * @return Size of the serialized state in bits.
         */
        static constexpr std::uint32_t BitSize();

        /**
         * @brief Property accessor;
         * @return Current imtq mode.
         */
        devices::imtq::Mode Mode() const noexcept;

        /**
         * @brief Property accessor;
         * @return Current last seen imtq error.
         */
        std::uint8_t ErrorCode() const noexcept;

        /**
         * @brief Property accessor;
         * @return Current flag indicating whether the imtq configuration has been updated recently.
         */
        bool ConfigurationUpdated() const noexcept;

        /**
         * @brief Property accessor;
         * @return Current imtq uptime.
         */
        std::chrono::seconds Uptime() const noexcept;

      private:
        devices::imtq::Mode mode;
        std::uint8_t errorCode;
        bool configurationUpdated;
        std::chrono::seconds uptime;
    };

    inline devices::imtq::Mode ImtqState::Mode() const noexcept
    {
        return this->mode;
    }

    inline std::uint8_t ImtqState::ErrorCode() const noexcept
    {
        return this->errorCode;
    }

    inline bool ImtqState::ConfigurationUpdated() const noexcept
    {
        return this->configurationUpdated;
    }

    inline std::chrono::seconds ImtqState::Uptime() const noexcept
    {
        return this->uptime;
    }

    constexpr std::uint32_t ImtqState::BitSize()
    {
        return 2 +                                      // BitLength<decltype(mode)>,
            BitLength<decltype(errorCode)> +            //
            BitLength<decltype(configurationUpdated)> + //
            BitLength<std::uint32_t>;
    }

    static_assert(ImtqState::BitSize() == 43, "Invalid serialized size");
}

#endif
