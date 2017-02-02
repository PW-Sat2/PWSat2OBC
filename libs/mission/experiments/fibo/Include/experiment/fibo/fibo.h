#ifndef LIBS_MISSION_EXPERIMENTS_FIBO_INCLUDE_EXPERIMENT_FIBO_FIBO_H_
#define LIBS_MISSION_EXPERIMENTS_FIBO_INCLUDE_EXPERIMENT_FIBO_FIBO_H_

#include <cstdint>
#include "fs/fs.h"
#include "mission/experiments.h"

namespace experiment
{
    namespace fibo
    {
        /**
         * @defgroup experiment_fibo Fibonacci experiment
         * @ingroup experiments
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
        class FibonacciExperiment final : public mission::experiments::IExperiment
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

            virtual mission::experiments::ExperimentCode Type() override;
            virtual mission::experiments::StartResult Start() override;
            virtual mission::experiments::IterationResult Iteration() override;
            virtual void Stop(mission::experiments::IterationResult lastResult) override;

            /**
             * @brief Experiment code
             */
            static constexpr mission::experiments::ExperimentCode Code = 0x01;

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
