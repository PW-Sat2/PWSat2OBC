#include "AdcsCoordinator.hpp"
#include "base/hertz.hpp"
#include "base/os.h"
#include "logger/logger.h"

namespace adcs
{
    using namespace std::chrono_literals;
    using services::time::ICurrentTime;

    AdcsCoordinator::AdcsCoordinator(IAdcsProcessor& builtinDetumbling_) //
        : currentMode(AdcsMode::Stopped),                                //
          _task("Adcs coordinator", this, TaskEntry)
    {
        std::uninitialized_fill(this->adcsMasks.begin(), this->adcsMasks.end(), false);
        adcsProcessors[static_cast<int>(AdcsMode::BuiltinDetumbling)] = &builtinDetumbling_;
    }

    OSResult AdcsCoordinator::Initialize()
    {
        auto result = this->_queue.Create();
        if (OS_RESULT_FAILED(result))
        {
            return result;
        }

        result = this->_task.Create();
        if (OS_RESULT_FAILED(result))
        {
            return result;
        }

        for (auto adcsProcessor : adcsProcessors)
        {
            result = adcsProcessor->Initialize();
            if (OS_RESULT_FAILED(result))
            {
                return result;
            }
        }

        return result;
    }

    AdcsMode AdcsCoordinator::CurrentMode() const
    {
        return this->currentMode;
    }

    OSResult AdcsCoordinator::EnableBuiltinDetumbling()
    {
        return RequestMode(AdcsMode::BuiltinDetumbling);
    }

    OSResult AdcsCoordinator::Stop()
    {
        return RequestMode(AdcsMode::Stopped);
    }

    OSResult AdcsCoordinator::Disable()
    {
        return RequestMode(AdcsMode::Disabled);
    }

    OSResult AdcsCoordinator::RequestMode(AdcsMode mode)
    {
        return this->_queue.Push(mode, 5s);
    }

    bool AdcsCoordinator::Disable(AdcsMode mode)
    {
        if (num(mode) < 0)
        {
            return true;
        }

        return OS_RESULT_SUCCEEDED(this->adcsProcessors[num(mode)]->Disable());
    }

    std::pair<AdcsMode, bool> AdcsCoordinator::SwitchMode(AdcsMode requestedMode)
    {
        const auto previousMode = this->currentMode.load();
        if (previousMode == requestedMode)
        {
            return std::make_pair(requestedMode, false);
        }

        switch (previousMode)
        {
            case AdcsMode::BuiltinDetumbling:
                this->currentMode = AdcsMode::Stopped;
                if (OS_RESULT_FAILED(this->adcsProcessors[num(previousMode)]->Disable()))
                {
                    LOGF(LOG_LEVEL_ERROR, "Unable to stop %d adcs mode.", static_cast<int>(previousMode));
                    return std::make_pair(AdcsMode::Stopped, true);
                }

                break;

            case AdcsMode::Stopped:
                break;

            case AdcsMode::Disabled:
            default:
                if (requestedMode != AdcsMode::Stopped)
                {
                    return std::make_pair(previousMode, false);
                }

                break;
        }

        switch (requestedMode)
        {
            case AdcsMode::BuiltinDetumbling:
            {
                if (IsModeBlocked(requestedMode))
                {
                    LOGF(LOG_LEVEL_ERROR, "%d adcs mode is blocked.", static_cast<int>(requestedMode));
                    return std::make_pair(this->currentMode.load(), previousMode != this->currentMode.load());
                }

                const auto status = this->adcsProcessors[static_cast<int>(requestedMode)]->Enable();
                if (OS_RESULT_SUCCEEDED(status))
                {
                    this->currentMode = requestedMode;
                }
                else
                {
                    LOGF(LOG_LEVEL_ERROR, "Unable to start %d adcs mode.", static_cast<int>(requestedMode));
                }

                return std::make_pair(this->currentMode.load(), previousMode != this->currentMode.load());
            }

            case AdcsMode::Disabled:
            case AdcsMode::Stopped:
                this->currentMode = requestedMode;
                return std::make_pair(requestedMode, true);

            default:
                return std::make_pair(this->currentMode.load(), previousMode != this->currentMode.load());
        }
    }

    std::chrono::milliseconds AdcsCoordinator::Run(AdcsMode mode, const std::chrono::milliseconds& startTime)
    {
        LOGF(LOG_LEVEL_TRACE, "[ADCS] Running ADCS loop. Mode: %d", static_cast<int>(mode));

        auto adcsProcessor = this->adcsProcessors[static_cast<int>(mode)];
        adcsProcessor->Process();

        const auto elapsed = System::GetUptime() - startTime;
        return std::max(0ms, adcsProcessor->GetWait() - elapsed);
    }

    void AdcsCoordinator::Loop()
    {
        AdcsMode mode = AdcsMode::Stopped;
        auto timeout = 0ms;

        auto nextIterationAt = System::GetUptime();
        for (;;)
        {
            AdcsMode newMode;
            if (OS_RESULT_SUCCEEDED(this->_queue.Pop(newMode, timeout)))
            {
                const auto result = SwitchMode(newMode);
                mode = result.first;
                if (result.second)
                {
                    nextIterationAt = System::GetUptime();
                }
            }

            switch (mode)
            {
                case AdcsMode::Stopped:
                case AdcsMode::Disabled:
                default:
                    timeout = InfiniteTimeout;
                    break;

                case AdcsMode::BuiltinDetumbling:
                    if (System::GetUptime() >= nextIterationAt)
                    {
                        timeout = Run(mode, nextIterationAt);
                        nextIterationAt = System::GetUptime() + timeout;
                    }
                    else
                    {
                        timeout = std::max(0ms, nextIterationAt - System::GetUptime());
                    }

                    break;
            }
        }
    }

    void AdcsCoordinator::TaskEntry(AdcsCoordinator* context)
    {
        context->Loop();
    }

    void AdcsCoordinator::SetBlockMode(AdcsMode adcsMode, bool isBlocked)
    {
        const auto index = num(adcsMode);
        if (index < 0 || static_cast<size_t>(index) >= this->adcsMasks.size())
        {
            return;
        }

        this->adcsMasks[index] = isBlocked;
    }

    bool AdcsCoordinator::IsModeBlocked(AdcsMode mode) const
    {
        const auto index = num(mode);
        if (index < 0 || static_cast<size_t>(index) >= this->adcsMasks.size())
        {
            return true;
        }

        return this->adcsMasks[index];
    }
}
