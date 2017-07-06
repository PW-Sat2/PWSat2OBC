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
        /**
         * @brief Class that contains antenna deployment activation counts
         * @remark Part of the antenna telemetry
         * @telemetry_element
         * @ingroup antenna
         */
        class ActivationCounts
        {
          public:
            /** ctor. */
            ActivationCounts();

            /**
             * @brief ctor.
             * @param antenna1 Number of deployment activations for first antenna.
             * @param antenna2 Number of deployment activations for second antenna.
             * @param antenna3 Number of deployment activations for third antenna.
             * @param antenna4 Number of deployment activations for fourth antenna.
             */
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

            /**
             * @brief Returns activation count of queried antenna
             * @param antenna Queried antenna identifier
             * @return Deployment activation count.
             */
            std::uint8_t GetActivationCount(AntennaId antenna) const;

            /**
             * @brief Updates number of deployment activations of specified antenna
             * @param antenna Identifier of antenna whose deployment counter should be updated.
             * @param count New deployment count value.
             */
            void SetActivationCount(AntennaId antenna, std::uint8_t count);

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

        /**
         * @brief Class that contains antenna deployment activation times
         * @remark Part of the antenna telemetry
         * @ingroup antenna
         */
        class ActivationTimes
        {
          public:
            /**
             * @brief Size in bits of single serialized field that contains antenna deployment activation time.
             */
            static constexpr std::uint32_t TimeLength = 12;

            /** ctor. */
            ActivationTimes();

            /**
             * @brief ctor.
             * @param antenna1 The cumulative antenna 1 deployment activation time.
             * @param antenna2 The cumulative antenna 2 deployment activation time.
             * @param antenna3 The cumulative antenna 3 deployment activation time.
             * @param antenna4 The cumulative antenna 4 deployment activation time.
             */
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

            /**
             * @brief Returns cumulative deployment activation time of queried antenna
             * @param antenna Queried antenna identifier
             * @return Cumulative deployment activation time.
             */
            std::chrono::seconds GetActivationTime(AntennaId antenna) const;

            /**
             * @brief Updates cumulative deployment activation time of specified antenna
             * @param antenna Identifier of antenna whose deployment time should be updated.
             * @param time New cumulative deployment activation time.
             */
            void SetActivationTime(AntennaId antenna, std::chrono::seconds time);

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

        /**
         * @brief This type represents telemetry of the antenna deployment subsystem.
         */
        class AntennaTelemetry
        {
          public:
            /**
             * @brief Antenna telemetry unique identifier.
             */
            static constexpr int Id = 7;

            /** ctor. */
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

            /**
             * @brief This function returns the object that contains total antenna deployment activation counts
             * one the queried channel.
             * @param channel Queried antenna controller
             * @return Reference to object with antenna deployment activation counts.
             */
            const ActivationCounts& GetActivationCounts(AntennaChannel channel) const;

            /**
             * @brief Updates telemetry with new antenna deployment activation counts for specified channel
             * @param channel Channel whose state should be updated.
             * @param counts New antenna deployment activation counts.
             */
            void SetActivationCounts(AntennaChannel channel, const ActivationCounts& counts);

            /**
             * @brief This function returns the object that contains cumulative antenna deployment times
             * one the queried channel.
             * @param channel Queried antenna controller
             * @return Reference to object with antenna deployment times.
             */
            const ActivationTimes& GetActivationTimes(AntennaChannel channel) const;

            /**
             * @brief Updates telemetry with new cumulative antenna deployment times for specified channel
             * @param channel Channel whose state should be updated.
             * @param times New antenna deployment times.
             */
            void SetActivationTimes(AntennaChannel channel, const ActivationTimes& times);

            /**
             * Updates deployment status of specific antenna as seen on the specified channel.
             * @param channel Channel that reported this status.
             * @param antenna Identifier of the antenna whose deployment status should be updated
             * @param status New deployment status.
             */
            void SetDeploymentStatus(AntennaChannel channel, AntennaId antenna, bool status);

            /**
             * @brief Queries requested antenna deployment status as seen on the specified channel.
             * @param channel Queried channel.
             * @param antenna Queried antenna identifier.
             * @return Requested antenna deployment status. True for being deployed false otherwise.
             */
            bool GetDeploymentStatus(AntennaChannel channel, AntennaId antenna) const;

            /**
             * @brief Requests combined antenna deployment status.
             * @return Combined antenna deployment status.
             */
            std::uint8_t GetDeploymentStatus() const;

            /**
             * @brief Updates combined antenna deployment status.
             * @param value New combined antenna deployment status.
             */
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
