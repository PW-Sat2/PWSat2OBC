#ifndef LIBS_DRIVERS_ANTENNA_TELEMETRY_HPP
#define LIBS_DRIVERS_ANTENNA_TELEMETRY_HPP

#pragma once

#include <array>
#include <bitset>
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
            return Collection().size() * 3;
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
            static constexpr std::uint32_t TimeLength = 8;

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

        enum class ChannelStatus
        {
            None = 0,
            IndependentBurn = 1 << 0,
            IgnoringSwitches = 1 << 1,
            Armed = 1 << 2
        };

        inline constexpr ChannelStatus operator|(const ChannelStatus a, const ChannelStatus b)
        {
            return static_cast<ChannelStatus>(num(a) | num(b));
        }

        inline constexpr ChannelStatus& operator|=(ChannelStatus& a, const ChannelStatus b)
        {
            return a = static_cast<ChannelStatus>(num(a) | num(b));
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

            void SetBurningStatus(AntennaChannel channel, bool antenna1, bool antenna2, bool antenna3, bool antenna4);
            bool GetBurningStatus(AntennaChannel channel, AntennaId antenna);

            void SetDeployedStatus(AntennaChannel channel, bool antenna1, bool antenna2, bool antenna3, bool antenna4);
            bool GetDeployedStatus(AntennaChannel channel, AntennaId antenna);

            void SetTimeReached(AntennaChannel channel, bool antenna1, bool antenna2, bool antenna3, bool antenna4);
            bool GetTimeReached(AntennaChannel channel, AntennaId antenna);

            void SetChannelStatus(AntennaChannel channel, ChannelStatus status);
            ChannelStatus GetChannelStatus(AntennaChannel channel) const;

          private:
            ActivationCounts activationCounts[2];
            ActivationTimes activationTimes[2];

            std::bitset<8> burnStatus;
            std::bitset<8> deployedStatus;
            std::bitset<8> timeReached;

            ChannelStatus channelStatuses[2];
        };

        constexpr std::uint32_t AntennaTelemetry::BitSize()
        {
            return 2 * (ActivationCounts::BitSize() + ActivationTimes::BitSize() + 4 + 4 + 4 + 3);
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

        inline bool AntennaTelemetry::GetBurningStatus(AntennaChannel channel, AntennaId antenna)
        {
            return this->burnStatus[4 * (channel - ANTENNA_FIRST_CHANNEL) + (antenna - AntennaId::ANTENNA1_ID)];
        }

        inline bool AntennaTelemetry::GetDeployedStatus(AntennaChannel channel, AntennaId antenna)
        {
            return this->deployedStatus[4 * (channel - ANTENNA_FIRST_CHANNEL) + (antenna - AntennaId::ANTENNA1_ID)];
        }

        inline bool AntennaTelemetry::GetTimeReached(AntennaChannel channel, AntennaId antenna)
        {
            return this->timeReached[4 * (channel - ANTENNA_FIRST_CHANNEL) + (antenna - AntennaId::ANTENNA1_ID)];
        }

        inline void AntennaTelemetry::SetChannelStatus(AntennaChannel channel, ChannelStatus status)
        {
            this->channelStatuses[channel - ANTENNA_FIRST_CHANNEL] = status;
        }

        inline ChannelStatus AntennaTelemetry::GetChannelStatus(AntennaChannel channel) const
        {
            return this->channelStatuses[channel - ANTENNA_FIRST_CHANNEL];
        }

        static_assert(AntennaTelemetry::BitSize() == 118, "Invalid telemetry size");

        /**
                * @brief Antenna telemetry provider
                * @ingroup mission_atenna
                */
        struct IAntennaTelemetryProvider
        {
            /**
             * @brief Fetches current antenna telemetry
             * @param telemetry Reference to object that will be filled with telemetry
             * @return true if telemetry was fetched correctly, false otherwise
             */
            virtual bool GetTelemetry(AntennaTelemetry& telemetry) const = 0;
        };
    }
}
#endif
