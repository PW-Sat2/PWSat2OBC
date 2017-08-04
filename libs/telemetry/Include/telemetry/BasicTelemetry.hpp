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
    /**
     * @brief Helper trait for SimpleTelemetryElement than is responsible for providing the default
     * value for requested object
     *
     * Specialize this template for types that require custom initialization.
     * @tparam T Requested type
     * @ingroup telemetry
     */
    template <typename T> struct Construct
    {
        /**
         * @brief Constructs default value for requested type.
         * @return Default value.
         */
        static constexpr T Default()
        {
            return T{};
        }
    };

    /**
     * @brief Specialization of helper trait SimpleTelemetryElement for arrays.
     *
     * @tparam T Type of array element.
     * @ingroup telemetry
     */
    template <typename T, size_t N> struct Construct<std::array<T, N>>
    {
        /**
         * @brief Constructs default value for requested type.
         * @return Default value.
         */
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
