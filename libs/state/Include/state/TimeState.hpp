#ifndef LIBS_STATE_TIME_STATE_HPP
#define LIBS_STATE_TIME_STATE_HPP

#pragma once

#include <chrono>
#include "base/fwd.hpp"

namespace state
{
    class TimeState
    {
      public:
        TimeState();

        TimeState(std::chrono::milliseconds missionTime, std::chrono::milliseconds externalTime);

        const std::chrono::milliseconds LastMissionTime() const;

        const std::chrono::milliseconds LastExternalTime() const;

        void Read(Reader& reader);

        void Write(Writer& writer) const;

        static constexpr std::uint32_t Size();

        bool operator==(const TimeState& arg) const;

        bool operator!=(const TimeState& arg) const;

      private:
        std::chrono::milliseconds lastMissionTime;
        std::chrono::milliseconds lastExternalTime;
    };

    inline const std::chrono::milliseconds TimeState::LastMissionTime() const
    {
        return this->lastMissionTime;
    }

    inline const std::chrono::milliseconds TimeState::LastExternalTime() const
    {
        return this->lastExternalTime;
    }

    constexpr std::uint32_t TimeState::Size()
    {
        return 2 * sizeof(decltype(std::declval<std::chrono::milliseconds>().count()));
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

#endif
