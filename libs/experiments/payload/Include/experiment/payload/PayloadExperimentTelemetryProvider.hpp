#ifndef LIBS_EXPERIMENTS_PAYLOAD_INCLUDE_EXPERIMENT_PAYLOAD_PAYLOADEXPERIMENTTELEMETRYPROVIDER_HPP_
#define LIBS_EXPERIMENTS_PAYLOAD_INCLUDE_EXPERIMENT_PAYLOAD_PAYLOADEXPERIMENTTELEMETRYPROVIDER_HPP_

#include "fs/ExperimentFile.hpp"

#include "telemetry/BasicTelemetry.hpp"
#include "telemetry/ErrorCounters.hpp"
#include "telemetry/Experiments.hpp"
#include "telemetry/SystemStartup.hpp"
#include "telemetry/TimeTelemetry.hpp"

#include "telemetry/Telemetry.hpp"

#include "eps/eps.h"
#include "obc/fdir.hpp"
#include "temp/mcu.hpp"

using namespace telemetry;

namespace experiment
{
    namespace payload
    {
        /**
         * @brief Interface for class providing parts of main telemetry used in Payload Commisioning Experiment.
         */
        struct IPayloadExperimentTelemetryProvider
        {
            /**
             * @brief Method saving telemetry to Experiment file.
             * @param file The experiment file.
             */
            virtual void Save(experiments::fs::ExperimentFile& file) = 0;
        };

        typedef telemetry::Telemetry<telemetry::ErrorCountingTelemetry, //
            telemetry::ExperimentTelemetry,                             //
            telemetry::McuTemperature,                                  //
            devices::eps::hk::ControllerATelemetry,                     //
            devices::eps::hk::ControllerBTelemetry                      //
            >
            ManagedTelemetryForPayloadExperiment;

        /**
         * @brief  Class providing parts of main telemetry used in Payload Commisioning Experiment.
         */
        class PayloadExperimentTelemetryProvider : public IPayloadExperimentTelemetryProvider
        {
          public:
            /**
             *  @brief Constructor
             *  @param epsProvider The provider of EPS telemetry.
             *  @param errorCounterProvider The provider of Error Counters telemetry.
             *  @param temperatureProvider The provider of MCU temperature.
             *  @param experimentProvider The provider of experiment telemetry.
             */
            PayloadExperimentTelemetryProvider(devices::eps::IEpsTelemetryProvider& epsProvider,
                error_counter::IErrorCountingTelemetryProvider* errorCounterProvider,
                temp::ITemperatureReader* temperatureProvider,
                experiments::IExperimentController* experimentProvider);

            virtual void Save(experiments::fs::ExperimentFile& file) override;

          private:
            ManagedTelemetryForPayloadExperiment _telemetry;

            devices::eps::IEpsTelemetryProvider& _epsProvider;
            error_counter::IErrorCountingTelemetryProvider* _errorCounterProvider;
            temp::ITemperatureReader* _temperatureProvider;
            experiments::IExperimentController* _experimentProvider;
        };
    }
}

#endif /* LIBS_EXPERIMENTS_PAYLOAD_INCLUDE_EXPERIMENT_PAYLOAD_PAYLOADEXPERIMENTTELEMETRYPROVIDER_HPP_ */
