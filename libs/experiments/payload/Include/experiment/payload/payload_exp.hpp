#ifndef LIBS_EXPERIMENTS_LEOP_INCLUDE_EXPERIMENT_PAYLOAD_PAYLOAD_EXP_HPP_
#define LIBS_EXPERIMENTS_LEOP_INCLUDE_EXPERIMENT_PAYLOAD_PAYLOAD_EXP_HPP_

#include <chrono>
#include "PayloadExperimentTelemetryProvider.hpp"
#include "experiments/experiments.h"
#include "fs/ExperimentFile.hpp"
#include "fs/fs.h"
#include "payload/interfaces.h"
#include "power/power.h"
#include "suns/suns.hpp"
#include "time/timer.h"

using PayloadTelemetry = devices::payload::PayloadTelemetry;

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
            static constexpr experiments::ExperimentCode Code = 0x6;

            /**
             * @brief Ctor
             * @param payload Payload driver
             * @param fileSystem File System provider
             * @param powerControl Power Control provider
             * @param time Current time provider
             * @param experimentalSunS Experimental SunS driver
             * @param epsProvider EPS telemetry provider
             * @param errorCounterProvider Error Counter telemetry provider
             * @param temperatureProvider MCU telemetry provider
             * @param experimentProvider Experiment telemetry provider
             */
            PayloadCommissioningExperiment(devices::payload::IPayloadDeviceDriver& payload,
                services::fs::IFileSystem& fileSystem,
                services::power::IPowerControl& powerControl,
                services::time::ICurrentTime& time,
                devices::suns::ISunSDriver& experimentalSunS,
                devices::eps::IEpsTelemetryProvider& epsProvider,
                error_counter::IErrorCountingTelemetryProvider* errorCounterProvider,
                temp::ITemperatureReader* temperatureProvider,
                experiments::IExperimentController* experimentProvider);

            /**
             * @brief Method allowing to set name of file where data will be saved.
             * @param fileName The name of file where data will be saved.
             */
            void SetOutputFile(const char* fileName);

            virtual experiments::ExperimentCode Type() override;
            virtual experiments::StartResult Start() override;
            virtual experiments::IterationResult Iteration() override;
            virtual void Stop(experiments::IterationResult lastResult) override;

          private:
            /** @brief Output file name. */
            static constexpr const char* DefaultFileName = "/payload";

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
            devices::payload::IPayloadDeviceDriver& _payload;

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

            PayloadExperimentTelemetryProvider _telemetryProvider;

            uint8_t _currentStep;

            char _fileName[30];
        };
    }
}

#endif /* LIBS_EXPERIMENTS_LEOP_INCLUDE_EXPERIMENT_PAYLOAD_PAYLOAD_EXP_HPP_ */
