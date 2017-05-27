#ifndef LIBS_OBC_EXPERIMENTS_INCLUDE_OBC_EXPERIMENTS_HPP_
#define LIBS_OBC_EXPERIMENTS_INCLUDE_OBC_EXPERIMENTS_HPP_

#include <array>
#include "experiment/adcs/adcs.hpp"
#include "experiment/fibo/fibo.h"
#include "experiments/experiments.h"
#include "fs/fs.h"

namespace obc
{
    /**
     * @defgroup obc_experiments Experiments
     * @ingroup obc
     *
     * @{
     */

    /**
     * @brief OBC experiments
     */
    class OBCExperiments
    {
      public:
        /**
         * @brief Ctor
         * @param fs File system
         * @param adcs ADCS coordinator
         * @param time Time provider
         */
        OBCExperiments(services::fs::IFileSystem& fs, adcs::IAdcsCoordinator& adcs, services::time::TimeProvider& time);

        /**
         * @brief Performs initialization
         */
        void InitializeRunlevel1();

        /** @brief Fibonacci experiment */
        experiment::fibo::FibonacciExperiment Fibo;

        /** @brief Detumbling experiment */
        experiment::adcs::DetumblingExperiment Detumbling;

        /** @brief List of all experiments */
        std::array<experiments::IExperiment*, 2> Experiments;

        /** @brief Experiments controller */
        experiments::ExperimentController ExperimentsController;
    };

    /** @} */
}

#endif /* LIBS_OBC_EXPERIMENTS_INCLUDE_OBC_EXPERIMENTS_HPP_ */
