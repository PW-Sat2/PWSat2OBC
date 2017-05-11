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
        ExperimentalSunPointing(devices::imtq::IImtqDriver& imtqDriver_);

        virtual OSResult Enable() override final;

        virtual OSResult Disable() override final;

        virtual void Process() override final;

        virtual std::chrono::hertz GetFrequency() const override final;

      private:
        devices::imtq::IImtqDriver& imtqDriver;
    };
}

#endif /* LIBS_ADCS_EXPERIMENTAL_ADCS_EXPERIMENTAL_SUNPOINTING_HPP */
