#include <cmath>
#include "base/reader.h"
#include "base/writer.h"
#include "time/TimeCorrectionConfiguration.hpp"
#include "time/TimeState.hpp"

using namespace std::chrono_literals;
using std::chrono::seconds;
using std::chrono::duration_cast;

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

    bool TimeState::IsDifferent(const TimeState& state) const
    {
        const auto missionDifference = duration_cast<seconds>(this->lastMissionTime - state.lastMissionTime);
        const auto externalDifference = duration_cast<seconds>(this->lastExternalTime - state.lastExternalTime);
        return std::abs(missionDifference.count()) >= 60 || std::abs(externalDifference.count()) >= 60;
    }

    TimeCorrectionConfiguration::TimeCorrectionConfiguration()
        : missionTimeFactor(1), //
          externalTimeFactor(1)
    {
    }

    TimeCorrectionConfiguration::TimeCorrectionConfiguration(std::int16_t missionTimeWeight, std::int16_t externalTimeWeight)
        : missionTimeFactor(missionTimeWeight), //
          externalTimeFactor(externalTimeWeight)
    {
    }

    void TimeCorrectionConfiguration::Read(Reader& reader)
    {
        this->missionTimeFactor = reader.ReadSignedWordLE();
        this->externalTimeFactor = reader.ReadSignedWordLE();
    }

    void TimeCorrectionConfiguration::Write(Writer& writer) const
    {
        writer.WriteSignedWordLE(this->missionTimeFactor);
        writer.WriteSignedWordLE(this->externalTimeFactor);
    }
}
