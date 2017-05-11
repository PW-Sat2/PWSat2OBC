#ifndef LIBS_ADCS_BUILTIN_DETUMBLING_HPP
#define LIBS_ADCS_BUILTIN_DETUMBLING_HPP

#pragma once

#include "adcs/adcs.hpp"

namespace adcs
{
    /**
     * @brief This class controlls primary adcs detumbling algorithm.
     * @ingroup adcs
     */
    class BuiltinDetumbling final : public IAdcsProcessor
    {
      public:
        virtual OSResult Enable() final override;

        virtual OSResult Disable() final override;

        virtual void Process() final override;

        virtual std::chrono::hertz GetFrequency() const final override;

      private:
        // TODO integrate imtq here
    };
}

#endif
