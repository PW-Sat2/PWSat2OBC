#ifndef LIBS_EXPERIMENTS_ADCS_INCLUDE_EXPERIMENT_ADCS_DATA_POINT_HPP_
#define LIBS_EXPERIMENTS_ADCS_INCLUDE_EXPERIMENT_ADCS_DATA_POINT_HPP_

#include "fs/ExperimentFile.hpp"
#include "gyro/telemetry.hpp"
#include "payload/telemetry.h"
#include "telemetry/BasicTelemetry.hpp"
#include "telemetry/fwd.hpp"

namespace experiment
{
    namespace adcs
    {
        struct DetumblingDataPoint
        {
            std::chrono::milliseconds Timestamp;
            devices::payload::PayloadTelemetry::Temperatures Temperatures;
            devices::payload::PayloadTelemetry::Photodiodes Photodiodes;
            devices::gyro::GyroscopeTelemetry Gyro;
            telemetry::ImtqMagnetometerMeasurements Magnetometer;
            telemetry::ImtqDipoles Dipoles;
            devices::payload::PayloadTelemetry::SunsRef ReferenceSunS;

            void WriteTo(experiments::fs::ExperimentFile& file);
        };
    }
}

#endif /* LIBS_EXPERIMENTS_ADCS_INCLUDE_EXPERIMENT_ADCS_DATA_POINT_HPP_ */
