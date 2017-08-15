#ifndef LIBS_MISSION_EXPERIMENTS_FIBO_INCLUDE_EXPERIMENT_FIBO_FIBO_H_
#define LIBS_MISSION_EXPERIMENTS_FIBO_INCLUDE_EXPERIMENT_FIBO_FIBO_H_

#include <cstdint>
#include "experiments/experiments.h"
#include "fs/fs.h"

namespace experiment
{
    namespace fibo
    {
        /**
         * @defgroup experiment_fibo Fibonacci experiment
         * @ingroup experiments
         *
         * Experiment code: 0x08
         *
         * Simple experiment to test experiment infrastructure
         *
         * @{
         */

        /**
         * @brief Fibonacci calculator
         */
        class Fibonacci
        {
          public:
            /** @brief Ctor */
            Fibonacci();
            /**
             * @brief Advances by one strep
             */
            void Next();

            /**
             * @brief Current Fibonacci number
             * @return Current number
             */
            std::uint32_t Current() const;

          private:
            /** @brief Counter */
            std::uint32_t _counter;
            /** @brief Previous number*/
            std::uint32_t _prev;

            /** @brief Current number */
            std::uint32_t _current;
        };

        /**
         * @brief Fibonacci experiment
         */
        class FibonacciExperiment final : public experiments::IExperiment
        {
          public:
            /**
             * @brief Ctor
             * @param fileSystem File system
             */
            FibonacciExperiment(services::fs::IFileSystem& fileSystem);

            /**
             * @brief Sets number of iterations to perform
             * @param iterations Number of iterations
             */
            void Iterations(std::uint32_t iterations);

            virtual experiments::ExperimentCode Type() override;
            virtual experiments::StartResult Start() override;
            virtual experiments::IterationResult Iteration() override;
            virtual void Stop(experiments::IterationResult lastResult) override;

            /**
             * @brief Experiment code
             */
            static constexpr experiments::ExperimentCode Code = 0x08;

          private:
            /** @brief File system */
            services::fs::IFileSystem& _fileSystem;
            /** @brief File with results */
            services::fs::File _file;

            /** @brief Fibonacci calculator */
            Fibonacci _fibo;

            /** @brief Number of iterations to perform */
            std::uint32_t _iterations;
        };

        /** @} */
    }
}

#endif /* LIBS_MISSION_EXPERIMENTS_FIBO_INCLUDE_EXPERIMENT_FIBO_FIBO_H_ */
