#ifndef LIBS_TELEMETRY_ERROR_COUNTERS_HPP
#define LIBS_TELEMETRY_ERROR_COUNTERS_HPP

#pragma once

#include <array>
#include <cstdint>
#include "base/fwd.hpp"
#include "error_counter/error_counter.hpp"

namespace telemetry
{
    /**
     * @brief This type represents telemetry element related to
     * state of hardware error counters.
     * @telemetry_element
     * @ingroup telemetry
     */
    class ErrorCountingTelemetry
    {
      public:
        /**
         * @brief Type of collection that contains all currently used error counters.
         */
        typedef std::array<std::uint8_t, error_counter::ErrorCounting::MaxDevices> Container;

        /**
         * @brief ErrorCountingTelemetry telemetry unique identifier.
         */
        static constexpr std::uint32_t Id = 4;

        /**
         * @brief ctor.
         */
        ErrorCountingTelemetry();

        /*
         * @brief ctor.
         * @param[in] state Current error counters' values
         */
        ErrorCountingTelemetry(const Container& state);

        /**
         * @brief Write the system error counting element to passed buffer writer object.
         * @param[in] writer Buffer writer object that should be used to write the serialized state.
         */
        void Write(BitWriter& writer) const;

        /**
         * @brief Returns size of the serialized state in bits.
         * @return Size of the serialized state in bits.
         */
        static constexpr std::uint32_t BitSize();

      private:
        /**
         * @brief current error counters' values.
         */
        Container array;
    };

    constexpr std::uint32_t ErrorCountingTelemetry::BitSize()
    {
        return 8 * Container().size() * sizeof(Container::value_type);
    }
}

#endif
