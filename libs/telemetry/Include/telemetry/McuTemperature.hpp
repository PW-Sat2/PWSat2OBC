#ifndef LIBS_TELEMETRY_INCLUDE_TELEMETRY_MCUTEMPERATURE_HPP_
#define LIBS_TELEMETRY_INCLUDE_TELEMETRY_MCUTEMPERATURE_HPP_

#pragma once

#include <cstdint>
#include "base/fwd.hpp"
#include "utils.h"

namespace telemetry
{
    /**
     * @brief This type represents telemetry element related mcu temperature.
     * @telemetry_element
     * @ingroup telemetry
     */
    class McuTemperature
    {
      public:
        /**
         * @brief Type that represents raw temperature reading.
         */
        typedef BitValue<std::uint16_t, 12> TemperatureType;

        /**
         * @brief McuTemperature telemetry unique identifier.
         */
        static constexpr int Id = 10;

        /**
         * @brief ctor.
         */
        McuTemperature() = default;

        /**
         * @brief ctor.
         * @param value Current raw mcu temperature value.
         */
        McuTemperature(TemperatureType value);

        /**
         * @brief Returns current raw mcu temperature value.
         * @return Raw mcu temperature value.
         */
        TemperatureType Temperature() const noexcept;

        /**
         * @brief Write the object to passed buffer writer object.
         * @param[in] writer Buffer writer object that should be used to write the serialized state.
         */
        void Write(BitWriter& writer) const;

        /**
         * @brief Returns size of the serialized state in bits.
         * @return Size of the serialized state in bits.
         */
        static constexpr std::uint32_t BitSize();

      private:
        BitValue<std::uint16_t, 12> temperature;
    };

    inline McuTemperature::TemperatureType McuTemperature::Temperature() const noexcept
    {
        return this->temperature;
    }

    constexpr std::uint32_t McuTemperature::BitSize()
    {
        return decltype(temperature)::Size;
    }

    static_assert(McuTemperature::BitSize() == 12, "Invalid serialized size");
}

#endif /* LIBS_TELEMETRY_INCLUDE_TELEMETRY_MCUTEMPERATURE_HPP_ */
