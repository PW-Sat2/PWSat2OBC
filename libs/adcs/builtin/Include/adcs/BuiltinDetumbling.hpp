#ifndef LIBS_ADCS_BUILTIN_DETUMBLING_HPP
#define LIBS_ADCS_BUILTIN_DETUMBLING_HPP

#pragma once

#include "adcs/adcs.hpp"
#include "base/hertz.hpp"
#include "imtq/imtq.h"

namespace adcs
{
    /**
     * @brief This class controlls primary adcs detumbling algorithm.
     * @ingroup adcs
     */
    class BuiltinDetumbling final : public IAdcsProcessor
    {
      public:
        BuiltinDetumbling(devices::imtq::IImtqDriver& imtqDriver_);

        virtual OSResult Enable() final override;

        virtual OSResult Disable() final override;

        virtual void Process() final override;

        virtual std::chrono::hertz GetFrequency() const final override;

      private:
        devices::imtq::IImtqDriver& imtqDriver;
    };
}

#endif
