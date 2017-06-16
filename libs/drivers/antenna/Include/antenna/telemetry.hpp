#ifndef LIBS_DRIVERS_ANTENNA_TELEMETRY_HPP
#define LIBS_DRIVERS_ANTENNA_TELEMETRY_HPP

#pragma once

#include <array>
#include <chrono>
#include <cstdint>
#include "antenna.h"
#include "base/fwd.hpp"
#include "gsl/span"
#include "system.h"
#include "utils.h"

namespace devices
{
    namespace antenna
    {
        class ActivationCounts
        {
          public:
            ActivationCounts();

            ActivationCounts(std::uint8_t antenna1, std::uint8_t antenna2, std::uint8_t antenna3, std::uint8_t antenna4);

            /**
             * @brief Write the antenna telemetry to passed buffer writer object.
             * @param[in] writer Buffer writer object that should be used to write the serialized state.
             */
            void Write(BitWriter& writer) const;

            /**
             * @brief Returns size of the serialized state in bits.
             *  @return Size of the serialized state in bits.
             */
            static constexpr std::uint32_t BitSize();

            std::uint8_t GetActivationCount(AntennaId antenna) const;

            void SetActivationCount(AntennaId antenna, std::uint8_t count);

            gsl::span<std::uint8_t> GetActivationCounts();

            gsl::span<const std::uint8_t> GetActivationCounts() const;

          private:
            typedef std::array<std::uint8_t, 4> Collection;
            Collection counts;
        };

        constexpr std::uint32_t ActivationCounts::BitSize()
        {
            return Collection().size() * BitLength<Collection::value_type>;
        }

        inline std::uint8_t ActivationCounts::GetActivationCount(AntennaId antenna) const
        {
            return this->counts[antenna - ANTENNA1_ID];
        }

        inline void ActivationCounts::SetActivationCount(AntennaId antenna, std::uint8_t count)
        {
            this->counts[antenna - ANTENNA1_ID] = count;
        }

        inline gsl::span<std::uint8_t> ActivationCounts::GetActivationCounts()
        {
            return gsl::make_span(this->counts);
        }

        inline gsl::span<const std::uint8_t> ActivationCounts::GetActivationCounts() const
        {
            return gsl::make_span(this->counts);
        }

        class ActivationTimes
        {
          public:
            static constexpr std::uint8_t TimeLength = 12;

            ActivationTimes();

            ActivationTimes(std::chrono::seconds antenna1, //
                std::chrono::seconds antenna2,             //
                std::chrono::seconds antenna3,             //
                std::chrono::seconds antenna4              //
                );
            /**
             * @brief Write the antenna telemetry to passed buffer writer object.
             * @param[in] writer Buffer writer object that should be used to write the serialized state.
             */
            void Write(BitWriter& writer) const;

            /**
             * @brief Returns size of the serialized state in bits.
             *  @return Size of the serialized state in bits.
             */
            static constexpr std::uint32_t BitSize();

            std::chrono::seconds GetActivationTime(AntennaId antenna) const;

            void SetActivationTime(AntennaId antenna, std::chrono::seconds time);

            gsl::span<std::chrono::seconds> GetActivationTimes();

            gsl::span<const std::chrono::seconds> GetActivationTimes() const;

          private:
            typedef std::array<std::chrono::seconds, 4> Collection;
            Collection times;
        };

        constexpr std::uint32_t ActivationTimes::BitSize()
        {
            return Collection().size() * TimeLength;
        }

        inline std::chrono::seconds ActivationTimes::GetActivationTime(AntennaId antenna) const
        {
            return this->times[antenna - ANTENNA1_ID];
        }

        inline void ActivationTimes::SetActivationTime(AntennaId antenna, std::chrono::seconds time)
        {
            this->times[antenna - ANTENNA1_ID] = time;
        }

        inline gsl::span<std::chrono::seconds> ActivationTimes::GetActivationTimes()
        {
            return gsl::make_span(this->times);
        }

        inline gsl::span<const std::chrono::seconds> ActivationTimes::GetActivationTimes() const
        {
            return gsl::make_span(this->times);
        }

        class AntennaTelemetry
        {
          public:
            /**
             * @brief Antenna telemetry unique identifier.
             */
            static constexpr int Id = 9;

            AntennaTelemetry();

            /**
             * @brief Write the antenna telemetry to passed buffer writer object.
             * @param[in] writer Buffer writer object that should be used to write the serialized state.
             */
            void Write(BitWriter& writer) const;

            /**
             * @brief Returns size of the serialized state in bits.
             * @return Size of the serialized state in bits.
             */
            static constexpr std::uint32_t BitSize();

            const ActivationCounts& GetActivationCounts(AntennaChannel channel) const;

            void SetActivationCounts(AntennaChannel channel, const ActivationCounts& counts);

            const ActivationTimes& GetActivationTimes(AntennaChannel channel) const;

            void SetActivationTimes(AntennaChannel channel, const ActivationTimes& times);

            void SetDeploymentStatus(AntennaChannel channel, AntennaId antenna, bool status);

            bool GetDeploymentStatus(AntennaChannel channel, AntennaId antenna) const;

            std::uint8_t GetDeploymentStatus() const;

            void SetDeploymentStatus(std::uint8_t value);

          private:
            std::uint8_t deploymentStatus;
            ActivationCounts activationCounts[2];
            ActivationTimes activationTimes[2];
        };

        constexpr std::uint32_t AntennaTelemetry::BitSize()
        {
            return BitLength<std::uint8_t> + 2 * ActivationCounts::BitSize() + 2 * ActivationTimes::BitSize();
        }

        inline const ActivationCounts& AntennaTelemetry::GetActivationCounts(AntennaChannel channel) const
        {
            return this->activationCounts[channel - ANTENNA_FIRST_CHANNEL];
        }

        inline void AntennaTelemetry::SetActivationCounts(AntennaChannel channel, const ActivationCounts& counts)
        {
            this->activationCounts[channel - ANTENNA_FIRST_CHANNEL] = counts;
        }

        inline const ActivationTimes& AntennaTelemetry::GetActivationTimes(AntennaChannel channel) const
        {
            return this->activationTimes[channel - ANTENNA_FIRST_CHANNEL];
        }

        inline void AntennaTelemetry::SetActivationTimes(AntennaChannel channel, const ActivationTimes& times)
        {
            this->activationTimes[channel - ANTENNA_FIRST_CHANNEL] = times;
        }

        inline std::uint8_t AntennaTelemetry::GetDeploymentStatus() const
        {
            return this->deploymentStatus;
        }

        inline void AntennaTelemetry::SetDeploymentStatus(std::uint8_t value)
        {
            this->deploymentStatus = value;
        }

        static_assert(AntennaTelemetry::BitSize() == 168, "Invalid telemetry size");
    }
}
#endif
