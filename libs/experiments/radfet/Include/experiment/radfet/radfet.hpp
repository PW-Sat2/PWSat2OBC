#ifndef LIBS_EXPERIMENTS_RADFET_INCLUDE_EXPERIMENT_RADFET_RADFET_HPP_
#define LIBS_EXPERIMENTS_RADFET_INCLUDE_EXPERIMENT_RADFET_RADFET_HPP_

#include <chrono>
#include "experiments/experiments.h"
#include "fs/ExperimentFile.hpp"
#include "fs/fwd.hpp"
#include "payload/interfaces.h"
#include "power/fwd.hpp"
#include "time/fwd.hpp"

namespace experiment
{
    namespace radfet
    {
        /**
         * @brief Interface for setting up RadFET experiment
         * @ingroup experiments
         */
        struct ISetupRadFETExperiment
        {
            /**
             * @brief Setups experiment
             * @param[in] delay Delay between sampling
             * @param[in] samplesCount Quantity of samples
             * @param[in] outputFile File name for storing measurements
             */
            virtual void Setup(std::chrono::seconds delay, uint8_t samplesCount, const char* outputFile) = 0;
        };

        /**
         * @brief Radiation Sensitive Field Effect Transistor (RadFET) experiment
         * @ingroup experiments
         */
        class RadFETExperiment final : public experiments::IExperiment, public ISetupRadFETExperiment
        {
          public:
            /** @brief Experiment code */
            static constexpr experiments::ExperimentCode Code = 0x5;

            /**
             * @brief Ctor
             * @param[in] fs Reference to file system object
             * @param[in] pld Reference to payload device driver
             * @param[in] powerControl Reference to power controller
             * @param[in] currentTime Reference to time provider
             */
            RadFETExperiment(services::fs::IFileSystem& fs,
                devices::payload::IPayloadDeviceDriver& pld,
                services::power::IPowerControl& powerControl,
                services::time::ICurrentTime& currentTime);

            virtual experiments::ExperimentCode Type() override;
            virtual experiments::StartResult Start() override;
            virtual experiments::IterationResult Iteration() override;
            virtual void Stop(experiments::IterationResult lastResult) override;
            virtual void Setup(std::chrono::seconds delay, uint8_t samplesCount, const char* outputFile) override;

          private:
            static bool SaveTimestamp(experiments::fs::ExperimentFile& file, std::chrono::milliseconds timestamp);

            static bool SaveStartTelemetry(experiments::fs::ExperimentFile& file,
                devices::payload::PayloadTelemetry::Radfet& telemetry,
                std::chrono::milliseconds timestamp);

            static bool SaveEndTelemetry(experiments::fs::ExperimentFile& file,
                devices::payload::PayloadTelemetry::Radfet& telemetry,
                std::chrono::milliseconds timestamp);

            static bool SaveLoopTelemetry(experiments::fs::ExperimentFile& file,
                devices::payload::PayloadTelemetry::Radfet& radfetTelemetry,
                devices::payload::PayloadTelemetry::Temperatures& temeraturesTelemetry,
                std::chrono::milliseconds timestamp);

            /**
             * @brief Saves RadFET registers to file
             * @param[in] file Experiment file to write to
             * @param[in] telemetry RadFET registers
             * @return True when save was successful, false otherwise
             */
            static bool SaveRadFETRegisters(experiments::fs::ExperimentFile& file,
                devices::payload::PayloadTelemetry::Radfet& telemetry,
                experiments::fs::ExperimentFile::PID pid);

            static bool SaveTemperatures(
                experiments::fs::ExperimentFile& file, devices::payload::PayloadTelemetry::Temperatures& telemetry);

            /**
             * @brief Saves RadFET registers to file
             * @param[in] writer Data writer
             * @param[in] telemetry RadFET registers
             */
            static void WriteRadFETRegisters(Writer& writer, devices::payload::PayloadTelemetry::Radfet& telemetry);

            /**
             * @brief Saves RadFET registers to file
             * @param[in] writer Data writer
             * @param[in] telemetry Temperatures
             */
            static void WriteTemperatures(Writer& writer, devices::payload::PayloadTelemetry::Temperatures& telemetry);

            /** @brief Reference to file system object */
            services::fs::IFileSystem& fs;

            /** @brief Reference to payload device driver */
            devices::payload::IPayloadDeviceDriver& pld;

            /** @brief Reference to power controller */
            services::power::IPowerControl& powerControl;

            /** @brief Reference to time provider */
            services::time::ICurrentTime& currentTime;

            /** @brief Data set of measurements */
            experiments::fs::ExperimentFile dataSet;

            /** @brief Quantity of collected samples */
            uint8_t samplesCollected;

            /** @brief Delay between sampling */
            std::chrono::seconds delay;

            /** @brief Requested samples count */
            uint8_t samplesCount;

            /** @brief File name for storing measurements */
            const char* outputFileName;
        };
    }
}

#endif /* LIBS_EXPERIMENTS_RADFET_INCLUDE_EXPERIMENT_RADFET_RADFET_HPP_ */
