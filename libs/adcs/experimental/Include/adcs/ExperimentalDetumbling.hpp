#ifndef LIBS_ADCS_EXPERIMENTAL_ADCS_EXPERIMENTAL_HPP
#define LIBS_ADCS_EXPERIMENTAL_ADCS_EXPERIMENTAL_HPP

#include "DetumblingComputations.hpp"
#include "adcs/adcs.hpp"
#include "base/hertz.hpp"
#include "imtq/imtq.h"

namespace adcs
{
    /**
     * @brief Experimental detumbling.
     */
    class ExperimentalDetumbling final : public IAdcsProcessor
    {
      public:
        ExperimentalDetumbling(devices::imtq::IImtqDriver& imtqDriver_);

        virtual OSResult Enable() override final;

        virtual OSResult Disable() override final;

        virtual void Process() override final;

        virtual std::chrono::hertz GetFrequency() const override final;

        static constexpr std::chrono::hertz Frequency{0.8};

      private:
        DetumblingComputations detumblingComputations;

        DetumblingComputations::State detumblingState;

        devices::imtq::IImtqDriver& imtqDriver;
    };
}

#endif /* LIBS_ADCS_EXPERIMENTAL_ADCS_EXPERIMENTAL_HPP */
