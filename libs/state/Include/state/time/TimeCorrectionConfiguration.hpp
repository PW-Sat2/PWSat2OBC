#ifndef LIBS_STATE_TIME_TIME_CORRECTION_CONFIGURATION_HPP
#define LIBS_STATE_TIME_TIME_CORRECTION_CONFIGURATION_HPP

#pragma once

#include <cstdint>

namespace state
{
    class TimeCorrectionConfiguration
    {
      public:
        TimeCorrectionConfiguration();

        TimeCorrectionConfiguration(std::uint16_t missionTimeWeight, std::uint16_t externalTimeWeight);

        std::uint16_t MissionTimeFactor() const noexcept;

        std::uint16_t ExternalTimeFactor() const noexcept;

        std::uint32_t Total() const noexcept;

        /**
         * @brief Read the time subsystem state from passed reader.
         * @param[in] reader Buffer reader that should be used to read the serialized state.
         */
        void Read(Reader& reader);

        /**
         * @brief Write the time state to passed buffer writer object.
         * @param[in] writer Buffer writer object that should be used to write the serialized state
         * of the time subsystem state.
         */
        void Write(Writer& writer) const;

        /**
         * @brief Returns size of the serialized state in bytes.
         * @return Size of the serialized state in bytes.
         */
        static constexpr std::uint32_t Size();

        /**
         * @brief Equality operator.
         * @param[in] arg Object to compare to.
         * @return Operation status.
         */
        bool operator==(const TimeCorrectionConfiguration& arg) const;

        /**
         * @brief Inequality operator.
         * @param[in] arg Object to compare to.
         * @return Operation status.
         */
        bool operator!=(const TimeCorrectionConfiguration& arg) const;

      private:
        std::uint16_t missionTimeFactor;
        std::uint16_t externalTimeFactor;
    };

    inline std::uint16_t TimeCorrectionConfiguration::MissionTimeFactor() const noexcept
    {
        return this->missionTimeFactor;
    }

    inline std::uint16_t TimeCorrectionConfiguration::ExternalTimeFactor() const noexcept
    {
        return this->externalTimeFactor;
    }

    inline std::uint32_t TimeCorrectionConfiguration::Total() const noexcept
    {
        return static_cast<std::uint32_t>(this->missionTimeFactor) + static_cast<std::uint32_t>(this->externalTimeFactor);
    }

    inline bool TimeCorrectionConfiguration::operator==(const TimeCorrectionConfiguration& arg) const
    {
        return this->missionTimeFactor == arg.missionTimeFactor && this->externalTimeFactor == arg.externalTimeFactor;
    }

    inline bool TimeCorrectionConfiguration::operator!=(const TimeCorrectionConfiguration& arg) const
    {
        return !(*this == arg);
    }

    constexpr std::uint32_t TimeCorrectionConfiguration::Size()
    {
        return sizeof(TimeCorrectionConfiguration::missionTimeFactor) + sizeof(TimeCorrectionConfiguration::externalTimeFactor);
    }
}

#endif
