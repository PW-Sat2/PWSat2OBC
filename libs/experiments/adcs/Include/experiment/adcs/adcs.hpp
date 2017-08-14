#ifndef LIBS_EXPERIMENTS_ADCS_INCLUDE_EXPERIMENT_ADCS_ADCS_HPP_
#define LIBS_EXPERIMENTS_ADCS_INCLUDE_EXPERIMENT_ADCS_ADCS_HPP_

#include <chrono>
#include "adcs/adcs.hpp"
#include "experiments/experiments.h"
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
             */
            DetumblingExperiment(::adcs::IAdcsCoordinator& adcs,
                services::time::ICurrentTime& time,
                services::power::IPowerControl& powerControl,
                devices::gyro::IGyroscopeDriver& gyro,
                devices::payload::IPayloadDeviceDriver& payload,
                telemetry::IImtqDataProvider& imtq);

            /**
             * @brief Sets experiment duration
             * @param duration Experiment duration
             */
            virtual void Duration(std::chrono::seconds duration) override;

            virtual experiments::ExperimentCode Type() override;
            virtual experiments::StartResult Start() override;
            virtual experiments::IterationResult Iteration() override;
            virtual void Stop(experiments::IterationResult lastResult) override;

            DetumblingDataPoint GatherSingleMeasurement();

          private:
            /** @brief ADCS coordinator */
            ::adcs::IAdcsCoordinator& _adcs;
            /** @brief Current time provider */
            services::time::ICurrentTime& _time;
            /** @brief Experiment duration */
            std::chrono::milliseconds _duration;
            /** @brief Point at time at which experiment should stop */
            std::chrono::milliseconds _endAt;
            /** @brief Power control */
            services::power::IPowerControl& _powerControl;
            /** @brief Gyroscope */
            devices::gyro::IGyroscopeDriver& _gyro;
            /** @brief Payload driver */
            devices::payload::IPayloadDeviceDriver& _payload;
            telemetry::IImtqDataProvider& _imtq;
        };
    }
}

#endif /* LIBS_EXPERIMENTS_ADCS_INCLUDE_EXPERIMENT_ADCS_ADCS_HPP_ */
