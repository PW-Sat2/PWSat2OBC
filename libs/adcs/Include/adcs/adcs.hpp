#ifndef LIBS_ADCS_HPP
#define LIBS_ADCS_HPP

#pragma once

#include "base/os.h"

namespace adcs
{
    enum class AdcsMode
    {
        Disabled,
        BuiltinDetumbling,
        CustomDetumbling,
        CustomSunpointing,
    };

    struct IDetumblingSupport
    {
        virtual OSResult EnableDetumbling() = 0;

        virtual OSResult DisableDetumbling() = 0;
    };

    struct ISunPointingSupport
    {
        virtual OSResult EnableSunPointing() = 0;

        virtual OSResult DisableSunPointing() = 0;
    };

    class IAdcsCoordinator
    {
        virtual AdcsMode CurrentMode() const = 0;

        virtual OSResult EnableBuiltinDetumbling() = 0;

        virtual OSResult EnableCustomDetumbling() = 0;

        virtual OSResult EnableSunPointing() = 0;

        virtual OSResult Disable() = 0;
    };
}

#endif
