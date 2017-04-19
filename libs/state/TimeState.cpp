#include "TimeState.hpp"
#include "base/reader.h"
#include "base/writer.h"
using namespace std::chrono_literals;

namespace state
{
    TimeState::TimeState()
        : lastMissionTime(0ms), //
          lastExternalTime(0ms)
    {
    }

    TimeState::TimeState(std::chrono::milliseconds missionTime, std::chrono::milliseconds externalTime)
        : lastMissionTime(missionTime), //
          lastExternalTime(externalTime)
    {
    }

    void TimeState::Read(Reader& reader)
    {
        auto value = reader.ReadQuadWordLE();
        this->lastMissionTime = std::chrono::milliseconds(value);
        value = reader.ReadQuadWordLE();
        this->lastExternalTime = std::chrono::milliseconds(value);
    }

    void TimeState::Write(Writer& writer) const
    {
        writer.WriteQuadWordLE(this->lastMissionTime.count());
        writer.WriteQuadWordLE(this->lastExternalTime.count());
    }
}
