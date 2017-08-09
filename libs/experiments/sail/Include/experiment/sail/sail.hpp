#ifndef LIBS_MISSION_EXPERIMENTS_SAIL_HPP
#define LIBS_MISSION_EXPERIMENTS_SAIL_HPP

#include <cstdint>
#include "experiments/experiments.h"
#include "fs/fs.h"

namespace experiment
{
    namespace sail
    {
        /**
         * @brief Sail experiment
         */
        class SailExperiment final : public experiments::IExperiment
        {
          public:
            /**
             * @brief Ctor
             * @param fileSystem File system
             */
            SailExperiment(services::fs::IFileSystem& fileSystem);

            virtual experiments::ExperimentCode Type() override;

            virtual experiments::StartResult Start() override;

            virtual experiments::IterationResult Iteration() override;

            virtual void Stop(experiments::IterationResult lastResult) override;

            /**
             * @brief Experiment code
             */
            static constexpr experiments::ExperimentCode Code = 0x07;

          private:
            /** @brief File system */
            services::fs::IFileSystem& _fileSystem;
        };
    }
}

#endif
