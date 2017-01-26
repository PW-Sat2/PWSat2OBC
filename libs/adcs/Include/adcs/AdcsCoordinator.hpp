#ifndef LIBS_ADCS_ADCS_COORDINATOR_HPP
#define LIBS_ADCS_ADCS_COORDINATOR_HPP

#pragma once

#include "adcs.hpp"
#include "base/os.h"

namespace adcs
{
    class AdcsCoordinator final : public IAdcsCoordinator
    {
      public:
        AdcsCoordinator(IDetumblingSupport& primaryDetembling, //
            IDetumblingSupport& secondaryDetumbling,           //
            ISunPointingSupport& primarySunPointing            //
            );

        virtual AdcsMode CurrentMode() const final override;

        virtual OSResult EnableBuiltinDetumbling() final override;

        virtual OSResult EnableCustomDetumbling() final override;

        virtual OSResult EnableSunPointing() final override;

        virtual OSResult Disable() final override;

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
}
#endif
