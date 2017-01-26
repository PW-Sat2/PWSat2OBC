#ifndef LIBS_MISSION_EXPERIMENTS_FIBO_INCLUDE_EXPERIMENT_FIBO_FIBO_H_
#define LIBS_MISSION_EXPERIMENTS_FIBO_INCLUDE_EXPERIMENT_FIBO_FIBO_H_

#include <cstdint>
#include "fs/fs.h"
#include "mission/experiments.h"

namespace experiment
{
    namespace fibo
    {
        class Fibonacci
        {
          public:
            Fibonacci();
            void Next();
            std::uint32_t Current() const;

          private:
            std::uint32_t _counter;
            std::uint32_t _prev;
            std::uint32_t _current;
        };

        class FibonacciExperiment final : public mission::experiments::IExperiment
        {
          public:
            FibonacciExperiment(services::fs::IFileSystem& fileSystem);

            virtual mission::experiments::Experiment Type() override;
            virtual void Run(mission::experiments::ExperimentContext& context) override;

          private:
            services::fs::IFileSystem& _fileSystem;
        };
    }
}

#endif /* LIBS_MISSION_EXPERIMENTS_FIBO_INCLUDE_EXPERIMENT_FIBO_FIBO_H_ */
