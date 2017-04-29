#ifndef LIBS_STATE_TIME_TIME_CORRECTION_CONFIGURATION_HPP
#define LIBS_STATE_TIME_TIME_CORRECTION_CONFIGURATION_HPP

#pragma once

#include <cstdint>

namespace state
{
    /**
     * @ingroup persistent_state
     * @brief This type represents time correction/synchronization configuration.
     *
     * It contains weights that are used in time correction weighted average formula.
     * The sum of all weights must not be zero.
     */
    class TimeCorrectionConfiguration
    {
      public:
        /**
         * @brief default ctor.
         *
         * Initializes this object with both weights set to 1.
         */
        TimeCorrectionConfiguration();

        /**
         * @brief ctor.
         * @param[in] missionTimeWeight Weight assigned to internal clock.
         * @param[in] externalTimeWeight Weight assigned to external clock.
         */
        TimeCorrectionConfiguration(std::int16_t missionTimeWeight, std::int16_t externalTimeWeight);

        /**
         * @brief Returns weight for internal clock.
         * @return Weight for internal clock.
         */
        std::int16_t MissionTimeFactor() const noexcept;

        /**
         * @brief Returns weight for external clock.
         * @return Weight for external clock.
         */
        std::int16_t ExternalTimeFactor() const noexcept;

        /**
         * @brief Returns sum of all weights.
         * @return Returns sum of all weights.
         */
        std::int32_t Total() const noexcept;

        /**
         * @brief Reads the configuration object from passed reader.
         * @param[in] reader Buffer reader that should be used to read the serialized state.
         */
        void Read(Reader& reader);

        /**
         * @brief Writes the configuration object to passed buffer writer object.
         * @param[in] writer Buffer writer object that should be used to write the serialized state
         * of the configuration object.
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
        /**
         * @brief Weight assigned to internal clock.
         */
        std::int16_t missionTimeFactor;

        /**
         * @brief Weight assigned to external clock.
         */
        std::int16_t externalTimeFactor;
    };

    inline std::int16_t TimeCorrectionConfiguration::MissionTimeFactor() const noexcept
    {
        return this->missionTimeFactor;
    }

    inline std::int16_t TimeCorrectionConfiguration::ExternalTimeFactor() const noexcept
    {
        return this->externalTimeFactor;
    }

    inline std::int32_t TimeCorrectionConfiguration::Total() const noexcept
    {
        return static_cast<std::int32_t>(this->missionTimeFactor) + static_cast<std::int32_t>(this->externalTimeFactor);
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
