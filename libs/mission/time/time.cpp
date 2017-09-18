#include "mission/time.hpp"
#include "logger/logger.h"
#include "state/struct.h"
#include "state/time/TimeCorrectionConfiguration.hpp"

using services::time::TimeProvider;
using namespace std::literals;

using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::duration_cast;

namespace mission
{
    constexpr milliseconds TimeTask::TimeCorrectionPeriod;

    constexpr milliseconds TimeTask::TimeCorrectionWarningThreshold;

    constexpr milliseconds TimeTask::MaximumTimeCorrection;

    TimeTask::TimeTask(std::tuple<TimeProvider&, devices::rtc::IRTC&, INotifyTimeChanged&> arguments)
        : provider(std::get<0>(arguments)),               //
          rtc(std::get<1>(arguments)),                    //
          syncSemaphore(System::CreateBinarySemaphore()), //
          _missionLoop(std::get<2>(arguments))
    {
    }

    UpdateResult TimeTask::UpdateProc(SystemState& state, void* param)
    {
        auto provider = static_cast<TimeProvider*>(param);
        auto currentTime = provider->GetCurrentTime();
        if (currentTime.HasValue)
        {
            state.Time = currentTime.Value;

            auto totalSeconds = duration_cast<seconds>(state.Time).count();
            auto seconds = static_cast<std::uint32_t>(totalSeconds % 60);
            auto minutes = static_cast<std::uint32_t>(totalSeconds / 60);
            auto hours = static_cast<std::uint32_t>(minutes / 60);
            auto days = static_cast<std::uint32_t>(hours / 24);

            minutes %= 60;
            hours %= 24;

            LOGF(LOG_LEVEL_INFO, "[time] Current mission time %02ld:%02ld:%02ld:%02ld", days, hours, minutes, seconds);

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
        System::GiveSemaphore(this->syncSemaphore);

        ActionDescriptor<SystemState> descriptor;
        descriptor.name = "Correct current time using external RTC";
        descriptor.param = this;
        descriptor.condition = CorrectTimeCondition;
        descriptor.actionProc = CorrectTimeProxy;
        return descriptor;
    }

    bool TimeTask::Lock(std::chrono::milliseconds timeout)
    {
        return OS_RESULT_SUCCEEDED(System::TakeSemaphore(this->syncSemaphore, timeout));
    }

    void TimeTask::Unlock()
    {
        System::GiveSemaphore(this->syncSemaphore);
    }

    bool TimeTask::CorrectTimeCondition(const SystemState& state, void* /*param*/)
    {
        state::TimeState timeState;
        if (!state.PersistentState.Get(timeState))
        {
            LOG(LOG_LEVEL_ERROR, "Can't get time state");
            return false;
        }

        return (state.Time - timeState.LastMissionTime()) >= TimeCorrectionPeriod;
    }

    void TimeTask::CorrectTimeProxy(SystemState& state, void* param)
    {
        TimeTask* This = static_cast<TimeTask*>(param);
        This->CorrectTime(state);
    }

    void TimeTask::CorrectTime(SystemState& state)
    {
        ::Lock lock(this->syncSemaphore, InfiniteTimeout);
        if (!lock())
        {
            LOG(LOG_LEVEL_ERROR, "Can't acquire time synchronization semaphore");
            return;
        }

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

        state::TimeState timeState;
        state::TimeCorrectionConfiguration timeCorrectionConfiguration;
        if (!state.PersistentState.Get(timeState))
        {
            LOG(LOG_LEVEL_ERROR, "Can't get time state");
            return;
        }

        if (!state.PersistentState.Get(timeCorrectionConfiguration))
        {
            LOG(LOG_LEVEL_ERROR, "Can't get time correction configuration");
            return;
        }

        auto newTime = PerformTimeCorrection(time.Value, currentRtcTime, timeState, timeCorrectionConfiguration);
        auto difference = newTime - time.Value;

        if (!provider.SetCurrentTime(newTime))
        {
            LOG(LOG_LEVEL_ERROR, "[Time] Unable to set corrected time");
            return;
        }

        if (!state.PersistentState.Set(state::TimeState(newTime, currentRtcTime)))
        {
            LOG(LOG_LEVEL_ERROR, "[Time] Unable to set time state");
        }

        state.Time = newTime;
        _missionLoop.NotifyTimeChanged(difference);
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

        if (deltaRtc == milliseconds::zero())
        {
            LOG(LOG_LEVEL_ERROR, "[Time] RTC reports 0 delta time. Ignored in correction.");
            return missionTime;
        }

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
