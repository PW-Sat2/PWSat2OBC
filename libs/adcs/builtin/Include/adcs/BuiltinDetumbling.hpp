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
        /**
         * @brief Ctor.
         *
         * @param[in] imtqDriver_ Low level imtq module driver.
         */
        BuiltinDetumbling(devices::imtq::IImtqDriver& imtqDriver_);

        virtual OSResult Initialize() final override;

        virtual OSResult Enable() final override;

        virtual OSResult Disable() final override;

        virtual void Process() final override;

        virtual std::chrono::milliseconds GetWait() const override final;

        /** @brief Algorithm refresh frequency. */
        static constexpr chrono_extensions::hertz Frequency = chrono_extensions::hertz{0.03};

      private:
        devices::imtq::IImtqDriver& imtqDriver;
    };
}

#endif
