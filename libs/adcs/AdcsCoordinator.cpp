#include "AdcsCoordinator.hpp"
#include "base/os.h"

namespace adcs
{
    AdcsCoordinator::AdcsCoordinator(IDetumblingSupport& builtinDetembling_, //
        IDetumblingSupport& customDetumbling_,                               //
        ISunPointingSupport& sunpointAlgorithm_)                             //
        : currentMode(AdcsMode::Disabled),                                   //
          builtinDetumbling(builtinDetembling_),                             //
          customDetumbling(customDetumbling_),                               //
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

    OSResult AdcsCoordinator::EnableCustomDetumbling()
    {
        if (this->currentMode == AdcsMode::CustomDetumbling)
        {
            return OSResult::Success;
        }

        auto result = Disable();
        if (OS_RESULT_FAILED(result))
        {
            return result;
        }

        return SetState(AdcsMode::CustomDetumbling, this->customDetumbling.EnableDetumbling());
    }

    OSResult AdcsCoordinator::EnableSunPointing()
    {
        if (this->currentMode == AdcsMode::CustomSunpointing)
        {
            return OSResult::Success;
        }

        auto result = Disable();
        if (OS_RESULT_FAILED(result))
        {
            return result;
        }

        return SetState(AdcsMode::CustomSunpointing, this->sunpointAlgorithm.EnableSunPointing());
    }

    OSResult AdcsCoordinator::Disable()
    {
        switch (this->currentMode)
        {
            case AdcsMode::BuiltinDetumbling:
                return SetState(AdcsMode::Disabled, this->builtinDetumbling.DisableDetumbling());
            case AdcsMode::CustomDetumbling:
                return SetState(AdcsMode::Disabled, this->customDetumbling.DisableDetumbling());
            case AdcsMode::CustomSunpointing:
                return SetState(AdcsMode::Disabled, this->sunpointAlgorithm.DisableSunPointing());
            default:
                return OSResult::Success;
        }
    }
}
