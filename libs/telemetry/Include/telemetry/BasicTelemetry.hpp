#ifndef LIBS_TELEMETRY_BASIC_TELEMETRY_HPP
#define LIBS_TELEMETRY_BASIC_TELEMETRY_HPP

#pragma once

#include <algorithm>
#include <cstdint>
#include <type_traits>
#include "base/BitWriter.hpp"
#include "utils.h"

namespace telemetry
{
    template <typename T> struct Construct
    {
        static constexpr T Default()
        {
            return T{};
        }
    };

    template <typename T, size_t N> struct Construct<std::array<T, N>>
    {
        static std::array<T, N> Default()
        {
            std::array<T, N> result;
            std::uninitialized_fill(result.begin(), result.end(), Construct<T>::Default());
            return result;
        }
    };

    /**
     * @brief This type represents telemetry element that contains single simple type as its value.
     * @telemetry_element
     * @ingroup telemetry
     */
    template <typename T, typename Tag> class SimpleTelemetryElement final
    {
      public:
        /**
         * @brief ctor.
         */
        constexpr SimpleTelemetryElement();

        /**
         * @brief ctor
         * @param value Initial telemetry value
         */
        constexpr SimpleTelemetryElement(T value);

        /**
         * @brief ctor
         * @param args Construction arguments
         */
        template <typename... Args> SimpleTelemetryElement(Args&&... args);

        /**
         * @brief Write the telemetry object to passed buffer writer object.
         * @param[in] writer Buffer writer object that should be used to write the serialized state.
         */
        void Write(BitWriter& writer) const;

        /**
         * @brief Returns size of the currently held value.
         * @return Currently held value.
         */
        T GetValue() const;

        /**
         * @brief Returns size of the serialized state in bits.
         *  @return Size of the serialized state in bits.
         */
        static constexpr std::uint32_t BitSize();

      private:
        T value;
    };

    template <typename T, typename Tag> constexpr SimpleTelemetryElement<T, Tag>::SimpleTelemetryElement() : value(Construct<T>::Default())
    {
    }

    template <typename T, typename Tag>
    constexpr SimpleTelemetryElement<T, Tag>::SimpleTelemetryElement(T initialValue) : value(initialValue)
    {
    }

    template <typename T, typename Tag>
    template <typename... Args>
    SimpleTelemetryElement<T, Tag>::SimpleTelemetryElement(Args&&... args) : value(std::forward<Args>(args)...)
    {
    }
    template <typename T, typename Tag> void SimpleTelemetryElement<T, Tag>::Write(BitWriter& writer) const
    {
        writer.Write(this->value);
    }

    template <typename T, typename Tag> inline T SimpleTelemetryElement<T, Tag>::GetValue() const
    {
        return this->value;
    }

    template <typename T, typename Tag> constexpr std::uint32_t SimpleTelemetryElement<T, Tag>::BitSize()
    {
        return BitLength<decltype(value)>;
    }
}

#endif
