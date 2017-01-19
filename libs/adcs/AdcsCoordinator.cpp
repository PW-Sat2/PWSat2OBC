#include "AdcsCoordinator.hpp"
#include "base/os.h"

namespace adcs
{
    AdcsCoordinator::AdcsCoordinator(IDetumblingSupport& primaryDetembling, //
        IDetumblingSupport& secondaryDetumbling,                            //
        ISunPointingSupport& primarySunPointing)                            //
        : currentMode(AdcsMode::Disabled),                                  //
          builtinDetumbling(primaryDetembling),                             //
          customDetumbling(secondaryDetumbling),                            //
          sunpointAlgorithm(primarySunPointing)
    {
    }

    OSResult AdcsCoordinator::EnableBuiltinDetumbling()
    {
        auto result = Disable();
        if (OS_RESULT_FAILED(result))
        {
            return result;
        }

        return SetState(AdcsMode::BuiltinDetumbling, this->builtinDetumbling.EnableDetumbling());
    }

    OSResult AdcsCoordinator::EnableCustomDetumbling()
    {
        auto result = Disable();
        if (OS_RESULT_FAILED(result))
        {
            return result;
        }

        return SetState(AdcsMode::CustomDetumbling, this->customDetumbling.EnableDetumbling());
    }

    OSResult AdcsCoordinator::EnableSunPointing()
    {
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
                return OSResult::InvalidOperation;
        }
    }
}
