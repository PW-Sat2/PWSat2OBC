#ifndef LIBS_EXPERIMENTS_LEOP_INCLUDE_EXPERIMENT_PAYLOAD_PAYLOAD_EXP_HPP_
#define LIBS_EXPERIMENTS_LEOP_INCLUDE_EXPERIMENT_PAYLOAD_PAYLOAD_EXP_HPP_

#include <chrono>
#include "experiments/experiments.h"
#include "fs/ExperimentFile.hpp"
#include "fs/fs.h"
#include "time/timer.h"

#include "payload/interfaces.h"
#include "power/power.h"
#include "suns/suns.hpp"

using PayloadTelemetry = drivers::payload::PayloadTelemetry;

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

            /**
             * @brief Ctor
             * @param gyro Gyroscope driver
             * @param time Current time provider
             * @param fileSystem File System provider
             */
            PayloadCommissioningExperiment(drivers::payload::IPayloadDeviceDriver& payload,
                services::fs::IFileSystem& fileSystem,
                services::power::IPowerControl& powerControl,
                services::time::ICurrentTime& time,
                devices::suns::ISunSDriver& experimentalSunS);

            virtual experiments::ExperimentCode Type() override;
            virtual experiments::StartResult Start() override;
            virtual experiments::IterationResult Iteration() override;
            virtual void Stop(experiments::IterationResult lastResult) override;

          private:
            experiments::IterationResult StartupStep();
            experiments::IterationResult SunSStep();
            experiments::IterationResult RadFETStep();
            experiments::IterationResult CamsStep();
            experiments::IterationResult CamsFullStep();

            void WriteTelemetry();
            void WriteRadFetTelemetry(PayloadTelemetry::Radfet& telemetry);
            void MeasureAndWritePayloadTemperaturesTelemetry();
            void MeasureAndWritePayloadStatusTelemetry();
            void MeasureAndWritePayloadHousekeepingTelemetry();
            void MeasureAndWritePayloadSunsTelemetry();
            void MeasureAndWritePayloadPhotodiodesTelemetry();
            void MeasureAndWriteExperimentalSunsTelemetry(uint8_t gain, uint8_t itime);

            /** @brief Payload driver */
            drivers::payload::IPayloadDeviceDriver& _payload;

            /** @brief Time provider */
            services::time::ICurrentTime& _time;

            /** @brief File System provider */
            services::fs::IFileSystem& _fileSystem;

            /** @brief Power Control Driver */
            services::power::IPowerControl& _powerControl;

            /** @brief SunS Driver */
            devices::suns::ISunSDriver& _experimentalSunS;

            /** @brief Experiment file with results */
            experiments::fs::ExperimentFile _experimentFile;

            uint8_t _currentStep;
        };
    }
}

#endif /* LIBS_EXPERIMENTS_LEOP_INCLUDE_EXPERIMENT_PAYLOAD_PAYLOAD_EXP_HPP_ */
