#ifndef LIBS_TELEMETRY_INCLUDE_TELEMETRY_EXPERIMENTS_HPP_
#define LIBS_TELEMETRY_INCLUDE_TELEMETRY_EXPERIMENTS_HPP_

#pragma once

#include <cstdint>
#include "base/fwd.hpp"
#include "experiments/experiments.h"
#include "utils.h"

namespace telemetry
{
    /**
     * @brief This type represents telemetry element related to the currenly performed experiment.
     * @telemetry_element
     * @ingroup telemetry
     */
    class ExperimentTelemetry
    {
      public:
        /**
         * @brief ExperimentTelemetry unique identifier.
         */
        static constexpr std::uint32_t Id = 11;

        /**
         * @brief ctor.
         */
        ExperimentTelemetry();

        /**
         * @brief ctor.
         * @param code Current experiment code
         * @param startResult Current experiment startup status
         * @param iterationResult Current experiment's last iteration result
         */
        ExperimentTelemetry(experiments::ExperimentCode code, //
            experiments::StartResult startResult,             //
            experiments::IterationResult iterationResult      //
            );

        /**
         * @brief ctor.
         * @param state Current expriment state.
         */
        ExperimentTelemetry(const experiments::ExperimentState& state);

        /**
         * @brief Returns current experiment code.
         * @return Current experiment code.
         */
        experiments::ExperimentCode CurrentExperiment() const noexcept;

        /**
         * @brief Returns current experiment startup status.
         * @return Current experiment startup status.
         */
        experiments::StartResult StartResult() const noexcept;

        /**
         * @brief Returns current experiment's last iteration result.
         * @return Current experiment's last iteration result.
         */
        experiments::IterationResult IterationResult() const noexcept;

        /**
         * @brief Write the object to passed buffer writer object.
         * @param[in] writer Buffer writer object that should be used to write the serialized state.
         */
        void Write(BitWriter& writer) const;

        /**
         * @brief Returns size of the serialized state in bits.
         * @return Size of the serialized state in bits.
         */
        static constexpr std::uint32_t BitSize();

      private:
        experiments::ExperimentCode currentExperiment;
        experiments::StartResult experimentStartResult;
        experiments::IterationResult experimentIterationResult;
    };

    inline experiments::ExperimentCode ExperimentTelemetry::CurrentExperiment() const noexcept
    {
        return this->currentExperiment;
    }

    inline experiments::StartResult ExperimentTelemetry::StartResult() const noexcept
    {
        return this->experimentStartResult;
    }

    inline experiments::IterationResult ExperimentTelemetry::IterationResult() const noexcept
    {
        return this->experimentIterationResult;
    }

    constexpr std::uint32_t ExperimentTelemetry::BitSize()
    {
        return Aggregate<decltype(currentExperiment), decltype(experimentStartResult), decltype(experimentIterationResult)>;
    }

    static_assert(ExperimentTelemetry::BitSize() == 24, "Invalid ExperimentTelemetry size");
}

#endif /* LIBS_TELEMETRY_INCLUDE_TELEMETRY_EXPERIMENTS_HPP_ */
