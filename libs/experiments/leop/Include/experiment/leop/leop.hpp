#ifndef LIBS_EXPERIMENTS_LEOP_INCLUDE_EXPERIMENT_LEOP_LEOP_HPP_
#define LIBS_EXPERIMENTS_LEOP_INCLUDE_EXPERIMENT_LEOP_LEOP_HPP_

#include <chrono>
#include "experiments/experiments.h"
#include "fs/ExperimentFile.hpp"
#include "fs/fs.h"
#include "gyro/gyro.h"
#include "time/timer.h"

namespace experiment
{
    namespace leop
    {
        /**
         * @brief Launch and Early Orbit Phase (LEOP) experiment
         * @ingroup experiments
         */
        class LaunchAndEarlyOrbitPhaseExperiment final : public experiments::IExperiment
        {
          public:
            /** @brief Experiment code */
            static constexpr experiments::ExperimentCode Code = 0x3;

            /** @brief Output file name. */
            static constexpr const char* FileName = "/leop.pwts";

            /** @brief Mission time when experiment should stop: T+1h */
            static constexpr std::chrono::milliseconds ExperimentTimeStop = std::chrono::milliseconds(60 * 60 * 1000);

            /**
             * @brief Ctor
             * @param gyro Gyroscope driver
             * @param time Current time provider
             * @param fileSystem File System provider
             */
            LaunchAndEarlyOrbitPhaseExperiment(
                devices::gyro::IGyroscopeDriver& gyro, services::time::ICurrentTime& time, services::fs::IFileSystem& fileSystem);

            virtual experiments::ExperimentCode Type() override;
            virtual experiments::StartResult Start() override;
            virtual experiments::IterationResult Iteration() override;
            virtual void Stop(experiments::IterationResult lastResult) override;

          private:
            /** @brief Gyroscope driver */
            devices::gyro::IGyroscopeDriver& _gyro;
            /** @brief Current time provider */
            services::time::ICurrentTime& _time;
            /** @brief File System provider */
            services::fs::IFileSystem& _fileSystem;

            /** @brief Experiment file with results */
            experiments::fs::ExperimentFile _experimentFile;

            experiments::IterationResult PerformMeasurements();
            experiments::IterationResult CheckExperimentTime();
        };
    }
}

#endif /* LIBS_EXPERIMENTS_LEOP_INCLUDE_EXPERIMENT_LEOP_LEOP_HPP_ */
