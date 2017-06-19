#ifndef LIBS_TELEMETRY_INCLUDE_TELEMETRY_TIMETELEMETRY_HPP_
#define LIBS_TELEMETRY_INCLUDE_TELEMETRY_TIMETELEMETRY_HPP_

#pragma once

#include <chrono>
#include "base/fwd.hpp"

namespace telemetry
{
    /**
     * @brief This type represents telemetry element related to state of internal time provider.
     * @telemetry_element
     * @ingroup telemetry
     */
    class InternalTimeTelemetry
    {
      public:
        /** ctor. */
        InternalTimeTelemetry();

        /**
         * @brief ctor.
         * @param newTime New value of the internal mission time.
         */
        InternalTimeTelemetry(std::chrono::milliseconds newTime);

        /**
         * @brief Current value of the current internal mission time.
         * @return Reference to the current mission time.
         */
        const std::chrono::milliseconds& Time() const;

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
        std::chrono::milliseconds time;
    };

    inline const std::chrono::milliseconds& InternalTimeTelemetry::Time() const
    {
        return this->time;
    }

    constexpr std::uint32_t InternalTimeTelemetry::BitSize()
    {
        return 64;
    }

    /**
     * @brief This type represents telemetry element related to state of external time provider.
     * @telemetry_element
     * @ingroup telemetry
     */
    class ExternalTimeTelemetry
    {
      public:
        /** ctor. */
        ExternalTimeTelemetry();

        /**
         * @brief ctor.
         * @param newTime New value of the external mission time.
         */
        ExternalTimeTelemetry(std::chrono::seconds newTime);

        /**
         * @brief Current value of the current external mission time.
         * @return Reference to the current mission time.
         */
        const std::chrono::seconds& Time() const;

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
        std::chrono::seconds time;
    };

    inline const std::chrono::seconds& ExternalTimeTelemetry::Time() const
    {
        return this->time;
    }

    constexpr std::uint32_t ExternalTimeTelemetry::BitSize()
    {
        return 32;
    }
}

#endif /* LIBS_TELEMETRY_INCLUDE_TELEMETRY_TIMETELEMETRY_HPP_ */
