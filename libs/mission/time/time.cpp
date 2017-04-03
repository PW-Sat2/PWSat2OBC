#include "Include/mission/time.hpp"
#include "logger/logger.h"
#include "state/struct.h"

using services::time::TimeProvider;
using namespace std::literals;

namespace mission
{
    TimeTask::TimeTask(std::tuple<TimeProvider&, devices::rtc::IRTC&> arguments)
        : provider(std::get<0>(arguments)), //
          rtc(std::get<1>(arguments))       //
    {
    }

    UpdateResult TimeTask::UpdateProc(SystemState& state, void* param)
    {
        auto timeProvider = static_cast<TimeProvider*>(param);
        Option<std::chrono::milliseconds> currentTime = timeProvider->GetCurrentTime();

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
        descriptor.actionProc = CorrectTime;
        return descriptor;
    }

    bool TimeTask::CorrectTimeCondition(const SystemState& state, void* param)
    {
        TimeTask* This = static_cast<TimeTask*>(param);

        return !This->lastTimeCorrection.HasValue || state.Time - This->lastTimeCorrection.Value >= TimeCorrectionPeriod;
    }

    void TimeTask::CorrectTime(const SystemState& /*state*/, void* param)
    {
        TimeTask* This = static_cast<TimeTask*>(param);

        auto time = This->provider.GetCurrentTime();
        This->lastTimeCorrection = time;

        if (!This->lastMissionTime.HasValue || !This->lastExternalClockTime.HasValue)
        {
            This->ReadInitialClockValues(time);
        }
        else
        {
            This->CorrectTime(time);
        }
    }

    void TimeTask::CorrectTime(const Option<std::chrono::milliseconds>& time)
    {
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

        auto deltaMcu = time.Value - lastMissionTime.Value;
        auto deltaRtc = rtcTime.ToDuration() - lastExternalClockTime.Value;

        deltaMcu = deltaMcu < 0ms ? 0ms : deltaMcu;
        deltaRtc = deltaRtc < 0s ? 0s : deltaRtc;

        auto correctedMissionTime = lastMissionTime.Value + (deltaMcu + deltaRtc) / 2;
        auto correctionValue = correctedMissionTime > time.Value ? correctedMissionTime - time.Value : time.Value - correctedMissionTime;

        if (correctionValue < MinimumTimeCorrection)
        {
            return;
        }

        if (correctionValue < MaximumTimeCorrection)
        {
            if (!provider.SetCurrentTime(correctedMissionTime))
            {
                LOG(LOG_LEVEL_ERROR, "[Time] Unable to set corrected time");
                return;
            }

            if (correctionValue > TimeCorrectionWarningThreshold)
            {
                LOGF(LOG_LEVEL_WARNING, "[Time] Large time correction value: %ld ms", static_cast<int32_t>(correctionValue.count()));
            }

            lastMissionTime = Some(correctedMissionTime);
        }
        else
        {
            lastMissionTime = time;
            LOG(LOG_LEVEL_ERROR, "[Time] To big correction value");
        }

        lastExternalClockTime = Some(rtcTime.ToDuration());
    }

    void TimeTask::ReadInitialClockValues(const Option<std::chrono::milliseconds>& time)
    {
        lastMissionTime = time;

        devices::rtc::RTCTime rtcTime;
        if (OS_RESULT_FAILED(rtc.ReadTime(rtcTime)))
        {
            LOG(LOG_LEVEL_ERROR, "Failed to retrieve initial time from external RTC");
            lastExternalClockTime = None<std::chrono::seconds>();
        }
        else
        {
            lastExternalClockTime = Some(rtcTime.ToDuration());
        }
    }
}
