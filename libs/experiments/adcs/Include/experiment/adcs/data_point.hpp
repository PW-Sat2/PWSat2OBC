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
        /**
         * @brief Detumbling experimet data point
         * @ingroup experiments
         */
        struct DetumblingDataPoint
        {
            /** @brief Data point timestmap */
            std::chrono::milliseconds Timestamp;
            /** @brief Temperatures */
            devices::payload::PayloadTelemetry::Temperatures Temperatures;
            /** @brief Photodiodes */
            devices::payload::PayloadTelemetry::Photodiodes Photodiodes;
            /** @brief Gyroscope */
            devices::gyro::GyroscopeTelemetry Gyro;
            /** @brief Magnetometer */
            telemetry::ImtqMagnetometerMeasurements Magnetometer;
            /** @brief Dipoles */
            telemetry::ImtqDipoles Dipoles;
            /** @brief Reference SunS */
            devices::payload::PayloadTelemetry::SunsRef ReferenceSunS;

            /**
             * @brief Writes data point to experiment file
             * @param file Experiment file
             */
            void WriteTo(experiments::fs::ExperimentFile& file);
        };
    }
}

#endif /* LIBS_EXPERIMENTS_ADCS_INCLUDE_EXPERIMENT_ADCS_DATA_POINT_HPP_ */
