#include "time/TimeState.hpp"
#include "base/reader.h"
#include "base/writer.h"
#include "time/TimeCorrectionConfiguration.hpp"

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

    TimeCorrectionConfiguration::TimeCorrectionConfiguration()
        : missionTimeFactor(1), //
          externalTimeFactor(1)
    {
    }

    TimeCorrectionConfiguration::TimeCorrectionConfiguration(std::uint16_t missionTimeWeight, std::uint16_t externalTimeWeight)
        : missionTimeFactor(missionTimeWeight), //
          externalTimeFactor(externalTimeWeight)
    {
    }

    void TimeCorrectionConfiguration::Read(Reader& reader)
    {
        this->missionTimeFactor = reader.ReadWordLE();
        this->externalTimeFactor = reader.ReadWordLE();
    }

    void TimeCorrectionConfiguration::Write(Writer& writer) const
    {
        writer.WriteWordLE(this->missionTimeFactor);
        writer.WriteWordLE(this->externalTimeFactor);
    }
}
