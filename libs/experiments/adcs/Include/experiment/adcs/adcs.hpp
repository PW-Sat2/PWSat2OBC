#ifndef LIBS_EXPERIMENTS_ADCS_INCLUDE_EXPERIMENT_ADCS_ADCS_HPP_
#define LIBS_EXPERIMENTS_ADCS_INCLUDE_EXPERIMENT_ADCS_ADCS_HPP_

#include <chrono>
#include "adcs/adcs.hpp"
#include "experiments/experiments.h"
#include "fs/ExperimentFile.hpp"
#include "gyro/fwd.hpp"
#include "payload/interfaces.h"
#include "power/fwd.hpp"
#include "telemetry/IImtqTelemetryCollector.hpp"
#include "time/timer.h"

namespace experiment
{
    namespace adcs
    {
        /**
         * @brief Interface for setting up detumbling experiment
         * @ingroup experiments
         *
         * Experiment code: 0x01
         */
        struct ISetupDetumblingExperiment
        {
            /**
             * @brief Sets experiment duration
             * @param duration Experiment duration
             */
            virtual void Duration(std::chrono::seconds duration) = 0;
            /**
             * @brief Sets sampling rate
             * @param interval Interval between samples
             */
            virtual void SampleRate(std::chrono::seconds interval) = 0;
        };

        struct DetumblingDataPoint;

        /**
         * @brief Detumbling experiment
         * @ingroup experiments
         */
        class DetumblingExperiment final : public experiments::IExperiment, public ISetupDetumblingExperiment
        {
          public:
            /** @brief Experiment code */
            static constexpr experiments::ExperimentCode Code = 0x1;

            /**
             * @brief Ctor
             * @param adcs ADCS coordinator
             * @param time Current time provider
             * @param powerControl Power control
             * @param gyro Gyroscope driver
             * @param payload Payload driver
             * @param imtq IMTQ data provider
             * @param fileSystem File system
             */
            DetumblingExperiment(::adcs::IAdcsCoordinator& adcs,
                services::time::ICurrentTime& time,
                services::power::IPowerControl& powerControl,
                devices::gyro::IGyroscopeDriver& gyro,
                devices::payload::IPayloadDeviceDriver& payload,
                telemetry::IImtqDataProvider& imtq,
                services::fs::IFileSystem& fileSystem);

            virtual void Duration(std::chrono::seconds duration) override;
            virtual void SampleRate(std::chrono::seconds interval) override;

            virtual experiments::ExperimentCode Type() override;
            virtual experiments::StartResult Start() override;
            virtual experiments::IterationResult Iteration() override;
            virtual void Stop(experiments::IterationResult lastResult) override;

            /**
             * @brief Gathers measurements from sensors and builds single data point
             * @return Data point
             */
            DetumblingDataPoint GatherSingleMeasurement();

          private:
            /**
             * @brief Reverts system to original state
             */
            void CleanUp();

            /** @brief ADCS coordinator */
            ::adcs::IAdcsCoordinator& _adcs;
            /** @brief Current time provider */
            services::time::ICurrentTime& _time;
            /** @brief Experiment duration */
            std::chrono::milliseconds _duration;
            std::chrono::seconds _sampleRate;
            /** @brief Point at time at which experiment should stop */
            std::chrono::milliseconds _endAt;
            /** @brief Power control */
            services::power::IPowerControl& _powerControl;
            /** @brief Gyroscope */
            devices::gyro::IGyroscopeDriver& _gyro;
            /** @brief Payload driver */
            devices::payload::IPayloadDeviceDriver& _payload;
            /** @brief IMTQ data provider */
            telemetry::IImtqDataProvider& _imtq;
            /** @brief File syste, */
            services::fs::IFileSystem& _fileSystem;
            /** @brief Experiment data file */
            experiments::fs::ExperimentFile _dataSet;
        };
    }
}

#endif /* LIBS_EXPERIMENTS_ADCS_INCLUDE_EXPERIMENT_ADCS_ADCS_HPP_ */
