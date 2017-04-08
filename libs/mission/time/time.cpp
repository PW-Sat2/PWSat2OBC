#include "Include/mission/time.hpp"
#include "logger/logger.h"
#include "state/struct.h"

using services::time::TimeProvider;
using namespace std::literals;

namespace mission
{
    constexpr std::chrono::milliseconds TimeTask::TimeCorrectionPeriod;

    constexpr std::chrono::milliseconds TimeTask::TimeCorrectionWarningThreshold;

    constexpr std::chrono::milliseconds TimeTask::MaximumTimeCorrection;

    TimeTask::TimeTask(std::tuple<TimeProvider&, devices::rtc::IRTC&> arguments)
        : provider(std::get<0>(arguments)), //
          rtc(std::get<1>(arguments))       //
    {
    }

    UpdateResult TimeTask::UpdateProc(SystemState& state, void* param)
    {
        auto task = static_cast<TimeTask*>(param);
        return task->UpdateSatelliteTime(state);
    }

    UpdateResult TimeTask::UpdateSatelliteTime(SystemState& state)
    {
        auto currentTime = provider.GetCurrentTime();
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
        descriptor.param = this;
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

        const auto currentRtcTime = rtcTime.ToDuration();
        auto newTime = PerformTimeCorrection(time.Value, currentRtcTime, state.PersistentState.Get<state::TimeState>());
        if (!provider.SetCurrentTime(newTime))
        {
            LOG(LOG_LEVEL_ERROR, "[Time] Unable to set corrected time");
            return;
        }

        state.PersistentState.Set(state::TimeState(newTime, currentRtcTime));
    }

    std::chrono::milliseconds TimeTask::PerformTimeCorrection(std::chrono::milliseconds missionTime, //
        std::chrono::milliseconds externalTime,                                                      //
        const state::TimeState& synchronizationState                                                 //
        )
    {
        auto deltaMcu = missionTime - synchronizationState.LastMissionTime();
        auto deltaRtc = externalTime - synchronizationState.LastExternalTime();
        deltaMcu = deltaMcu < 0ms ? 0ms : deltaMcu;
        deltaRtc = deltaRtc < 0s ? 0s : deltaRtc;

        auto correctedMissionTime = synchronizationState.LastMissionTime() + (deltaMcu + deltaRtc) / 2;
        auto correctionValue = correctedMissionTime > missionTime ? correctedMissionTime - missionTime : missionTime - correctedMissionTime;

        if (correctionValue < MaximumTimeCorrection)
        {
            if (correctionValue > TimeCorrectionWarningThreshold)
            {
                LOGF(LOG_LEVEL_WARNING, "[Time] Large time correction value: %ld ms", static_cast<int32_t>(correctionValue.count()));
            }

            return correctedMissionTime;
        }
        else
        {
            LOGF(LOG_LEVEL_ERROR, "[Time] To big correction value: %lld", correctedMissionTime.count());
            return missionTime;
        }
    }
}
