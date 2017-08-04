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

      private:
        devices::imtq::VoltageInMiliVolt digitalVoltage;
        devices::imtq::VoltageInMiliVolt analogVoltage;
        devices::imtq::Current digitalCurrent;
        devices::imtq::Current analogCurrent;
        devices::imtq::TemperatureMeasurement mcuTemperature;
    };

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
         * @param status Last seen imtq status
         * @param mode Current imtq mode
         * @param errorCode Last imtq error code
         * @param configurationUpdated Flag indicating whether imtq state has been recently updated
         * @param uptime Current imtq uptime
         */
        ImtqState(std::uint8_t status, //
            devices::imtq::Mode mode,
            std::uint8_t errorCode,
            bool configurationUpdated,
            std::chrono::seconds uptime);

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

      private:
        std::uint8_t status;
        devices::imtq::Mode mode;
        std::uint8_t errorCode;
        bool configurationUpdated;
        std::chrono::seconds uptime;
    };

    constexpr std::uint32_t ImtqState::BitSize()
    {
        return BitLength<decltype(status)> +            //
            2 +                                         // BitLength<decltype(mode)>,
            BitLength<decltype(errorCode)> +            //
            BitLength<decltype(configurationUpdated)> + //
            BitLength<std::uint32_t>;
    }

    static_assert(ImtqState::BitSize() == 51, "Invalid serialized size");
}

#endif
