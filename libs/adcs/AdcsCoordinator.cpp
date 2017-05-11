#include "AdcsCoordinator.hpp"
#include "base/hertz.hpp"
#include "base/os.h"
#include "logger/logger.h"

namespace adcs
{
    using namespace std::chrono_literals;
    using services::time::ICurrentTime;

    AdcsCoordinator::AdcsCoordinator(IAdcsProcessor& builtinDetumbling_, //
        IAdcsProcessor& experimentalDetumbling_,                         //
        IAdcsProcessor& sunpointAlgorithm_,                              //
        ICurrentTime& currentTime_)                                      //
        : taskHandle(nullptr),                                           //
          currentTime(currentTime_),                                     //
          sync(System::CreateBinarySemaphore()),                         //
          currentMode(AdcsMode::Disabled)                                //
    {
        adcsProcessors[static_cast<int>(AdcsMode::BuiltinDetumbling)] = &builtinDetumbling_;
        adcsProcessors[static_cast<int>(AdcsMode::ExperimentalDetumbling)] = &experimentalDetumbling_;
        adcsProcessors[static_cast<int>(AdcsMode::ExperimentalSunpointing)] = &sunpointAlgorithm_;
    }

    OSResult AdcsCoordinator::Initialize()
    {
        OSResult result;

        result = System::GiveSemaphore(this->sync);
        if (!OS_RESULT_SUCCEEDED(result))
        {
            return result;
        }

        result = System::CreateTask(TaskEntry, "ADCSTask", 2_KB, this, TaskPriority::P4, &this->taskHandle);
        if (!OS_RESULT_SUCCEEDED(result))
        {
            return result;
        }

        System::SuspendTask(taskHandle);

        for (auto adcsProcessor : adcsProcessors)
        {
            result = adcsProcessor->Initialize();
            if (!OS_RESULT_SUCCEEDED(result))
            {
                return result;
            }
        }

        return result;
    }

    AdcsMode AdcsCoordinator::CurrentMode() const
    {
        Lock lock(this->sync, InfiniteTimeout);

        if (!lock())
        {
            LOG(LOG_LEVEL_TRACE, "[ADCS] Unable to lock synchronization semaphore");
        }

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
        Lock lock(this->sync, InfiniteTimeout);

        if (!lock())
        {
            LOG(LOG_LEVEL_TRACE, "[ADCS] Unable to lock synchronization semaphore");
            return OSResult::InvalidOperation;
        }

        if (this->currentMode == AdcsMode::BuiltinDetumbling)
        {
            return OSResult::Success;
        }

        auto disableResult = Disable(false);
        if (OS_RESULT_FAILED(disableResult))
        {
            return disableResult;
        }

        auto enableResult =
            SetState(AdcsMode::BuiltinDetumbling, this->adcsProcessors[static_cast<int>(AdcsMode::BuiltinDetumbling)]->Enable());

        System::ResumeTask(taskHandle);

        return enableResult;
    }

    OSResult AdcsCoordinator::EnableExperimentalDetumbling()
    {
        Lock lock(this->sync, InfiniteTimeout);

        if (!lock())
        {
            LOG(LOG_LEVEL_TRACE, "[ADCS] Unable to lock synchronization semaphore");
            return OSResult::InvalidOperation;
        }

        if (this->currentMode == AdcsMode::ExperimentalDetumbling)
        {
            return OSResult::Success;
        }

        auto disableResult = Disable(false);
        if (OS_RESULT_FAILED(disableResult))
        {
            return disableResult;
        }

        auto enableResult =
            SetState(AdcsMode::ExperimentalDetumbling, this->adcsProcessors[static_cast<int>(AdcsMode::ExperimentalDetumbling)]->Enable());

        System::ResumeTask(taskHandle);

        return enableResult;
    }

    OSResult AdcsCoordinator::EnableSunPointing()
    {
        Lock lock(this->sync, InfiniteTimeout);

        if (!lock())
        {
            LOG(LOG_LEVEL_TRACE, "[ADCS] Unable to lock synchronization semaphore");
            return OSResult::InvalidOperation;
        }

        if (this->currentMode == AdcsMode::ExperimentalSunpointing)
        {
            return OSResult::Success;
        }

        auto disableResult = Disable(false);
        if (OS_RESULT_FAILED(disableResult))
        {
            return disableResult;
        }

        auto enableResult = SetState(
            AdcsMode::ExperimentalSunpointing, this->adcsProcessors[static_cast<int>(AdcsMode::ExperimentalSunpointing)]->Enable());

        System::ResumeTask(taskHandle);

        return enableResult;
    }

    OSResult AdcsCoordinator::Disable()
    {
        Lock lock(this->sync, InfiniteTimeout);

        if (!lock())
        {
            LOG(LOG_LEVEL_TRACE, "[ADCS] Unable to lock synchronization semaphore");
            return OSResult::InvalidOperation;
        }

        return Disable(true);
    }

    OSResult AdcsCoordinator::Disable(bool suspend)
    {
        if (this->currentMode == AdcsMode::Disabled)
        {
            return OSResult::Success;
        }

        auto result = SetState(AdcsMode::Disabled, this->adcsProcessors[static_cast<int>(this->currentMode)]->Disable());

        if (suspend)
        {
            System::SuspendTask(taskHandle);
        }

        return result;
    }

    void AdcsCoordinator::Loop()
    {
        if (this->currentMode == AdcsMode::Disabled)
        {
            System::SleepTask(1s);
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
        auto waitDuration = chrono_extensions::period_cast<std::chrono::milliseconds>(adcsProcessor->GetFrequency()) - elapsed;
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
