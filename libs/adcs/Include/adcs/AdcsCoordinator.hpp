#ifndef LIBS_ADCS_ADCS_COORDINATOR_HPP
#define LIBS_ADCS_ADCS_COORDINATOR_HPP

#pragma once

#include "adcs.hpp"
#include "base/os.h"

namespace adcs
{
    class AdcsCoordinator
    {
      public:
        AdcsCoordinator(IDetumblingSupport& primaryDetembling, //
            IDetumblingSupport& secondaryDetumbling,           //
            ISunPointingSupport& primarySunPointing            //
            );

        AdcsMode CurrentMode() const;

        OSResult EnableBuiltinDetumbling();

        OSResult EnableCustomDetumbling();

        OSResult EnableSunPointing();

        OSResult Disable();

      private:
        OSResult SetState(AdcsMode newMode, OSResult operationStatus);
        AdcsMode currentMode;
        IDetumblingSupport& builtinDetumbling;
        IDetumblingSupport& customDetumbling;
        ISunPointingSupport& sunpointAlgorithm;
    };

    inline AdcsMode AdcsCoordinator::CurrentMode() const
    {
        return this->currentMode;
    }

    inline OSResult AdcsCoordinator::SetState(AdcsMode newMode, OSResult operationStatus)
    {
        if (OS_RESULT_SUCCEEDED(operationStatus))
        {
            this->currentMode = newMode;
        }

        return operationStatus;
    }
}
#endif
