#include "AdcsCoordinator.hpp"
#include "base/os.h"

namespace adcs
{
    AdcsCoordinator::AdcsCoordinator(IDetumblingSupport& builtinDetembling_, //
        IDetumblingSupport& experimentalDetumbling_,                         //
        ISunPointingSupport& sunpointAlgorithm_)                             //
        : currentMode(AdcsMode::Disabled),                                   //
          builtinDetumbling(builtinDetembling_),                             //
          experimentalDetumbling(experimentalDetumbling_),                   //
          sunpointAlgorithm(sunpointAlgorithm_)
    {
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

        return SetState(AdcsMode::BuiltinDetumbling, this->builtinDetumbling.EnableDetumbling());
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

        return SetState(AdcsMode::ExperimentalDetumbling, this->experimentalDetumbling.EnableDetumbling());
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

        return SetState(AdcsMode::ExperimentalSunpointing, this->sunpointAlgorithm.EnableSunPointing());
    }

    OSResult AdcsCoordinator::Disable()
    {
        switch (this->currentMode)
        {
            case AdcsMode::BuiltinDetumbling:
                return SetState(AdcsMode::Disabled, this->builtinDetumbling.DisableDetumbling());
            case AdcsMode::ExperimentalDetumbling:
                return SetState(AdcsMode::Disabled, this->experimentalDetumbling.DisableDetumbling());
            case AdcsMode::ExperimentalSunpointing:
                return SetState(AdcsMode::Disabled, this->sunpointAlgorithm.DisableSunPointing());
            default:
                return OSResult::Success;
        }
    }
}
