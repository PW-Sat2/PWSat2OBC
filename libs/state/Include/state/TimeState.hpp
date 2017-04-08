#ifndef LIBS_STATE_TIME_STATE_HPP
#define LIBS_STATE_TIME_STATE_HPP

#pragma once

#include <chrono>
#include "base/fwd.hpp"

/**
 * @addtogroup StateDef
 * @{
 */
namespace state
{
    /**
     * @brief This type contains information related to time subsystem state, primarily synchronization between
     * internal clock and external real time clock.
     */
    class TimeState
    {
      public:
        /**
         * @brief ctor.
         */
        TimeState();

        /**
         * @brief ctor.
         * @param[in] missionTime Current mission time in milliseconds as seen on internal clock.
         * @param[in] externalTime Current mission time in milliseconds as seen on external clock.
         */
        TimeState(std::chrono::milliseconds missionTime, std::chrono::milliseconds externalTime);

        /**
         * @brief Returns current mission time in milliseconds as seen on internal clock
         * during last time synchronization proces.
         *
         * @return Current mission time in milliseconds as seen on internal clock.
         */
        const std::chrono::milliseconds& LastMissionTime() const;

        /**
         * @brief Returns current mission time in milliseconds as seen on external clock
         * during last time synchronization proces.
         *
         * @return Current mission time in milliseconds as seen on external clock.
         */
        const std::chrono::milliseconds& LastExternalTime() const;

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
        bool operator==(const TimeState& arg) const;

        /**
         * @brief Inequality operator.
         * @param[in] arg Object to compare to.
         * @return Operation status.
         */
        bool operator!=(const TimeState& arg) const;

      private:
        /**
         * @brief Current mission time in milliseconds as seen on internal clock.
         */
        std::chrono::milliseconds lastMissionTime;

        /**
         * @brief Current mission time in milliseconds as seen on external clock.
         */
        std::chrono::milliseconds lastExternalTime;
    };

    inline const std::chrono::milliseconds& TimeState::LastMissionTime() const
    {
        return this->lastMissionTime;
    }

    inline const std::chrono::milliseconds& TimeState::LastExternalTime() const
    {
        return this->lastExternalTime;
    }

    inline constexpr std::uint32_t TimeState::Size()
    {
        return sizeof(decltype(lastMissionTime)::rep) + sizeof(decltype(lastExternalTime)::rep);
    }

    inline bool TimeState::operator==(const TimeState& arg) const
    {
        return this->lastMissionTime == arg.lastMissionTime && this->lastExternalTime == arg.lastExternalTime;
    }

    inline bool TimeState::operator!=(const TimeState& arg) const
    {
        return !(*this == arg);
    }
}

/** @} */

#endif
