#ifndef LIBS_ADCS_EXPERIMENTAL_ADCS_EXPERIMENTAL_SUNPOINTING_HPP
#define LIBS_ADCS_EXPERIMENTAL_ADCS_EXPERIMENTAL_SUNPOINTING_HPP

#include "adcs/adcs.hpp"
#include "base/hertz.hpp"
#include "imtq/imtq.h"

namespace adcs
{
    /**
     * @brief Experimental sunpointing.
     */
    class ExperimentalSunPointing final : public IAdcsProcessor
    {
      public:
        /**
         * @brief Ctor.
         *
         * @param[in] imtqDriver_ Low level imtq module driver.
         */
        ExperimentalSunPointing(devices::imtq::IImtqDriver& imtqDriver_);

        virtual OSResult Initialize() override final;

        virtual OSResult Enable() override final;

        virtual OSResult Disable() override final;

        virtual void Process() override final;

        virtual chrono_extensions::hertz GetFrequency() const override final;

      private:
        /** @brief Low level imtq module driver. */
        devices::imtq::IImtqDriver& imtqDriver;
    };
}

#endif /* LIBS_ADCS_EXPERIMENTAL_ADCS_EXPERIMENTAL_SUNPOINTING_HPP */
