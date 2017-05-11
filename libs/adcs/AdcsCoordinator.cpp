#include "AdcsCoordinator.hpp"
#include "base/hertz.hpp"
#include "base/os.h"
#include "logger/logger.h"

namespace adcs
{
    using services::time::ICurrentTime;

    AdcsCoordinator::AdcsCoordinator(IAdcsProcessor& builtinDetumbling_, //
        IAdcsProcessor& experimentalDetumbling_,                         //
        IAdcsProcessor& sunpointAlgorithm_,                              //
        ICurrentTime& currentTime_)                                      //
        : taskHandle(nullptr),                                           //
          currentTime(currentTime_),                                     //
          currentMode(AdcsMode::Disabled)                                //
    {
        adcsProcessors[static_cast<int>(AdcsMode::BuiltinDetumbling)] = &builtinDetumbling_;
        adcsProcessors[static_cast<int>(AdcsMode::ExperimentalDetumbling)] = &experimentalDetumbling_;
        adcsProcessors[static_cast<int>(AdcsMode::ExperimentalSunpointing)] = &sunpointAlgorithm_;
    }

    OSResult AdcsCoordinator::Initialize()
    {
        return System::CreateTask(TaskEntry, "ADCSTask", 1024, this, TaskPriority::P4, &this->taskHandle);
    }

    AdcsMode AdcsCoordinator::CurrentMode() const
    {
        return this->currentMode;
    }

    OSResult AdcsCoordinator::SetState(AdcsMode newMode, OSResult operationStatus)
    {
        if (OS_RESULT_SUCCEEDED(operationStatus))
        {
            this->currentMode = newMode;
        }

        return operationStatus;
    }

    OSResult AdcsCoordinator::EnableBuiltinDetumbling()
    {
        if (this->currentMode == AdcsMode::BuiltinDetumbling)
        {
            return OSResult::Success;
        }

        auto result = Disable();
        if (OS_RESULT_FAILED(result))
        {
            return result;
        }

        return SetState(AdcsMode::BuiltinDetumbling, this->adcsProcessors[static_cast<int>(AdcsMode::BuiltinDetumbling)]->Enable());
    }

    OSResult AdcsCoordinator::EnableExperimentalDetumbling()
    {
        if (this->currentMode == AdcsMode::ExperimentalDetumbling)
        {
            return OSResult::Success;
        }

        auto result = Disable();
        if (OS_RESULT_FAILED(result))
        {
            return result;
        }

        return SetState(
            AdcsMode::ExperimentalDetumbling, this->adcsProcessors[static_cast<int>(AdcsMode::ExperimentalDetumbling)]->Enable());
    }

    OSResult AdcsCoordinator::EnableSunPointing()
    {
        if (this->currentMode == AdcsMode::ExperimentalSunpointing)
        {
            return OSResult::Success;
        }

        auto result = Disable();
        if (OS_RESULT_FAILED(result))
        {
            return result;
        }

        return SetState(
            AdcsMode::ExperimentalSunpointing, this->adcsProcessors[static_cast<int>(AdcsMode::ExperimentalSunpointing)]->Enable());
    }

    OSResult AdcsCoordinator::Disable()
    {
        if (this->currentMode == AdcsMode::Disabled)
        {
            return OSResult::Success;
        }

        return SetState(AdcsMode::Disabled, this->adcsProcessors[static_cast<int>(this->currentMode)]->Disable());
    }

    void AdcsCoordinator::Loop()
    {
        if (this->currentMode == AdcsMode::Disabled)
        {
            LOG(LOG_LEVEL_TRACE, "[ADCS] Running ADCS loop. No mode enabled");
            System::SleepTask(std::chrono::seconds(1));
            return;
        }

        LOGF(LOG_LEVEL_TRACE, "[ADCS] Running ADCS loop. Mode: %d", static_cast<int>(this->currentMode));

        auto now = this->currentTime.GetCurrentTime();
        if (!now.HasValue)
        {
            LOG(LOG_LEVEL_ERROR, "[ADCS] Current time not available");
            return;
        }

        auto beforeTime = now.Value;

        auto adcsProcessor = this->adcsProcessors[static_cast<int>(this->currentMode)];
        adcsProcessor->Process();

        now = this->currentTime.GetCurrentTime();
        if (!now.HasValue)
        {
            LOG(LOG_LEVEL_ERROR, "[ADCS] Current time not available");
            return;
        }

        auto elapsed = now.Value - beforeTime;
        auto waitDuration = std::chrono::period_cast<std::chrono::milliseconds>(adcsProcessor->GetFrequency()) - elapsed;
        if (waitDuration.count() > 0)
        {
            System::SleepTask(waitDuration);
        }
    }

    void AdcsCoordinator::TaskEntry(void* arg)
    {
        auto context = static_cast<AdcsCoordinator*>(arg);
        for (;;)
        {
            context->Loop();
        }
    }
}
