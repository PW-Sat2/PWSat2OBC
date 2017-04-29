#include "Include/mission/time.hpp"
#include "logger/logger.h"
#include "state/Time/TimeCorrectionConfiguration.hpp"
#include "state/struct.h"

using services::time::TimeProvider;
using namespace std::literals;

using std::chrono::milliseconds;
using std::chrono::duration_cast;

namespace mission
{
    constexpr milliseconds TimeTask::TimeCorrectionPeriod;

    constexpr milliseconds TimeTask::TimeCorrectionWarningThreshold;

    constexpr milliseconds TimeTask::MaximumTimeCorrection;

    TimeTask::TimeTask(std::tuple<TimeProvider&, devices::rtc::IRTC&> arguments)
        : provider(std::get<0>(arguments)), //
          rtc(std::get<1>(arguments))       //
    {
    }

    UpdateResult TimeTask::UpdateProc(SystemState& state, void* param)
    {
        auto provider = static_cast<TimeProvider*>(param);
        auto currentTime = provider->GetCurrentTime();
        if (currentTime.HasValue)
        {
            state.Time = currentTime.Value;
            return UpdateResult::Ok;
        }
        else
        {
            return UpdateResult::Warning;
        }
    }

    UpdateDescriptor<SystemState> TimeTask::BuildUpdate()
    {
        UpdateDescriptor<SystemState> descriptor;
        descriptor.name = "Time State Update";
        descriptor.updateProc = UpdateProc;
        descriptor.param = &this->provider;
        return descriptor;
    }

    ActionDescriptor<SystemState> TimeTask::BuildAction()
    {
        ActionDescriptor<SystemState> descriptor;
        descriptor.name = "Correct current time using external RTC";
        descriptor.param = this;
        descriptor.condition = CorrectTimeCondition;
        descriptor.actionProc = CorrectTimeProxy;
        return descriptor;
    }

    bool TimeTask::CorrectTimeCondition(const SystemState& state, void* /*param*/)
    {
        const auto& timeState = state.PersistentState.Get<state::TimeState>();
        return (state.Time - timeState.LastMissionTime()) >= TimeCorrectionPeriod;
    }

    void TimeTask::CorrectTimeProxy(const SystemState& state, void* param)
    {
        TimeTask* This = static_cast<TimeTask*>(param);
        This->CorrectTime(state);
    }

    void TimeTask::CorrectTime(const SystemState& state)
    {
        auto time = this->provider.GetCurrentTime();
        if (!time.HasValue)
        {
            LOG(LOG_LEVEL_ERROR, "Unable to correct: failed to retrieve current time");
            return;
        }

        devices::rtc::RTCTime rtcTime;
        if (OS_RESULT_FAILED(rtc.ReadTime(rtcTime)))
        {
            LOG(LOG_LEVEL_ERROR, "Failed to retrieve delta time from external RTC");
            return;
        }

        if (!rtcTime.IsValid())
        {
            LOG(LOG_LEVEL_ERROR, "RTC Time is invalid");
            return;
        }

        const auto currentRtcTime = duration_cast<milliseconds>(rtcTime.ToDuration());
        auto newTime = PerformTimeCorrection(time.Value,
            currentRtcTime,
            state.PersistentState.Get<state::TimeState>(),
            state.PersistentState.Get<state::TimeCorrectionConfiguration>());
        if (!provider.SetCurrentTime(newTime))
        {
            LOG(LOG_LEVEL_ERROR, "[Time] Unable to set corrected time");
            return;
        }

        state.PersistentState.Set(state::TimeState(newTime, currentRtcTime));
    }

    milliseconds TimeTask::PerformTimeCorrection(milliseconds missionTime, //
        milliseconds externalTime,                                         //
        const state::TimeState& synchronizationState,                      //
        const state::TimeCorrectionConfiguration& correctionConfiguation)
    {
        auto deltaMcu = missionTime - synchronizationState.LastMissionTime();
        auto deltaRtc = externalTime - synchronizationState.LastExternalTime();

        deltaMcu = deltaMcu < 0ms ? 0ms : deltaMcu;
        deltaRtc = deltaRtc < 0s ? 0s : deltaRtc;

        auto correctedMissionTime = synchronizationState.LastMissionTime() +
            (deltaMcu * correctionConfiguation.MissionTimeFactor() + deltaRtc * correctionConfiguation.ExternalTimeFactor()) /
                correctionConfiguation.Total();
        auto correctionValue = correctedMissionTime > missionTime ? correctedMissionTime - missionTime : missionTime - correctedMissionTime;

        if (correctionValue < MaximumTimeCorrection)
        {
            if (correctionValue > TimeCorrectionWarningThreshold)
            {
                LOGF(LOG_LEVEL_WARNING, "[Time] Large time correction value: %lu ms", static_cast<uint32_t>(correctionValue.count()));
            }

            return correctedMissionTime;
        }
        else
        {
            LOGF(LOG_LEVEL_ERROR, "[Time] To big correction value: %lu ms", static_cast<uint32_t>(correctedMissionTime.count()));
            return missionTime;
        }
    }
}
