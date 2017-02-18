#ifndef LIBS_EXPERIMENTS_ADCS_INCLUDE_EXPERIMENT_ADCS_ADCS_HPP_
#define LIBS_EXPERIMENTS_ADCS_INCLUDE_EXPERIMENT_ADCS_ADCS_HPP_

#include <chrono>
#include "adcs/adcs.hpp"
#include "experiments/experiments.h"
#include "time/timer.h"

namespace experiment
{
    namespace adcs
    {
        /**
         * @brief Detumbling experiment
         * @ingroup experiments
         */
        class DetumblingExperiment final : public experiments::IExperiment
        {
          public:
            /** @brief Experiment code */
            static constexpr experiments::ExperimentCode Code = 0x2;

            /**
             * @brief Ctor
             * @param adcs ADCS coordinator
             * @param time Current time provider
             */
            DetumblingExperiment(::adcs::IAdcsCoordinator& adcs, services::time::ICurrentTime& time);

            /**
             * @brief Sets experiment duration
             * @param duration Experiment duration
             */
            void Duration(std::chrono::seconds duration);

            virtual experiments::ExperimentCode Type() override;
            virtual experiments::StartResult Start() override;
            virtual experiments::IterationResult Iteration() override;
            virtual void Stop(experiments::IterationResult lastResult) override;

          private:
            /** @brief ADCS coordinator */
            ::adcs::IAdcsCoordinator& _adcs;
            /** @brief Current time provider */
            services::time::ICurrentTime& _time;
            /** @brief Experiment duration */
            std::chrono::milliseconds _duration;
            /** @brief Point at time at which experiment should stop */
            std::chrono::milliseconds _endAt;
        };
    }
}

#endif /* LIBS_EXPERIMENTS_ADCS_INCLUDE_EXPERIMENT_ADCS_ADCS_HPP_ */
