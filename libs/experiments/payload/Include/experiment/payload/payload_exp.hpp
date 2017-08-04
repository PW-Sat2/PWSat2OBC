#ifndef LIBS_EXPERIMENTS_LEOP_INCLUDE_EXPERIMENT_LEOP_LEOP_HPP_
#define LIBS_EXPERIMENTS_LEOP_INCLUDE_EXPERIMENT_LEOP_LEOP_HPP_

#include <chrono>
#include "experiments/experiments.h"
#include "fs/ExperimentFile.hpp"
#include "fs/fs.h"
#include "time/timer.h"

#include "eps/eps.h"
#include "payload/interfaces.h"

using EPSErrorCode = devices::eps::ErrorCode;

namespace experiment
{
    namespace payload
    {
        /**
         * @brief Payload Commissioning experiment
         * @ingroup experiments
         */
        class PayloadCommissioningExperiment final : public experiments::IExperiment
        {
          public:
            /** @brief Experiment code */
            static constexpr experiments::ExperimentCode Code = 0x4;

            /** @brief Output file name. */
            static constexpr const char* FileName = "/payload.pwts";

            /** @brief Mission time when experiment should stop: T+1h */
            static constexpr std::chrono::milliseconds ExperimentTimeStop = std::chrono::milliseconds(60 * 60 * 1000);

            /**
             * @brief Ctor
             * @param gyro Gyroscope driver
             * @param time Current time provider
             * @param fileSystem File System provider
             */
            PayloadCommissioningExperiment(drivers::payload::IPayloadDeviceDriver& payload,
                services::fs::IFileSystem& fileSystem,
                devices::eps::EPSDriver& eps,
                services::time::ICurrentTime& time);

            virtual experiments::ExperimentCode Type() override;
            virtual experiments::StartResult Start() override;
            virtual experiments::IterationResult Iteration() override;
            virtual void Stop(experiments::IterationResult lastResult) override;

          private:
            static constexpr int32_t SunSCurrentPassThreshold = 100;   // 100 mA
            static constexpr int32_t RadFETCurrentPassThreshold = 100; // 100 mA

            /** @brief Payload driver */
            drivers::payload::IPayloadDeviceDriver& _payload;
            /** @brief Time provider */
            services::time::ICurrentTime& _time;

            /** @brief File System provider */
            services::fs::IFileSystem& _fileSystem;
            /** @brief EPS Driver             */
            devices::eps::EPSDriver& _eps;

            /** @brief Experiment file with results */
            experiments::fs::ExperimentFile _experimentFile;

            experiments::IterationResult PerformMeasurements();
            experiments::IterationResult CheckExperimentTime();

            experiments::IterationResult SunSStep();
            experiments::IterationResult RadFETStep();

            void WriteEPSError(EPSErrorCode error);
        };
    }
}

#endif /* LIBS_EXPERIMENTS_LEOP_INCLUDE_EXPERIMENT_LEOP_LEOP_HPP_ */
