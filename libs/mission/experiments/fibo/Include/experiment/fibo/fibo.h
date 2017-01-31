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

            void Iterations(std::uint32_t iterations);

            virtual mission::experiments::Experiment Type() override;
            virtual mission::experiments::StartResult Start() override;
            virtual mission::experiments::IterationResult Iteration() override;
            virtual void Stop(mission::experiments::IterationResult lastResult) override;

          private:
            services::fs::IFileSystem& _fileSystem;
            services::fs::File _file;
            Fibonacci _fibo;

            std::uint32_t _iterations;
        };
    }
}

#endif /* LIBS_MISSION_EXPERIMENTS_FIBO_INCLUDE_EXPERIMENT_FIBO_FIBO_H_ */
