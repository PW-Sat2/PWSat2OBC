#ifndef LIBS_OBC_EXPERIMENTS_INCLUDE_OBC_EXPERIMENTS_HPP_
#define LIBS_OBC_EXPERIMENTS_INCLUDE_OBC_EXPERIMENTS_HPP_

#include <array>
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
         */
        OBCExperiments(services::fs::IFileSystem& fs);

        /**
         * @brief Performs initialization
         */
        void Initialize();

        /** @brief Fibonacci experiment */
        experiment::fibo::FibonacciExperiment Fibo;

        /** @brief List of all experiments */
        std::array<experiments::IExperiment*, 1> Experiments;

        /** @brief Experiments controller */
        experiments::ExperimentController ExperimentsController;
    };

    /** @} */
}

#endif /* LIBS_OBC_EXPERIMENTS_INCLUDE_OBC_EXPERIMENTS_HPP_ */
