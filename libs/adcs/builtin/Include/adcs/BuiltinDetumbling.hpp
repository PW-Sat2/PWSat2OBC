#ifndef LIBS_ADCS_BUILTIN_DETUMBLING_HPP
#define LIBS_ADCS_BUILTIN_DETUMBLING_HPP

#pragma once

#include "adcs/adcs.hpp"

namespace adcs
{
    class BuiltinDetumbling final : public IDetumblingSupport
    {
      public:
        virtual OSResult EnableDetumbling() final override;

        virtual OSResult DisableDetumbling() final override;

      private:
        // TODO integrate imtq here
    };
}

#endif
