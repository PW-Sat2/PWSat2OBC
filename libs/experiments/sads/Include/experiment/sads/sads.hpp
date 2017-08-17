#ifndef LIBS_MISSION_EXPERIMENTS_SADS_HPP
#define LIBS_MISSION_EXPERIMENTS_SADS_HPP

#pragma once

#include <cstdint>
#include "adcs/adcs.hpp"
#include "experiments/experiments.h"
#include "fs/ExperimentFile.hpp"
#include "fs/fs.h"
#include "gpio/forward.h"
#include "gyro/fwd.hpp"
#include "mission/sads_fwd.hpp"
#include "payload/interfaces.h"
#include "photo/fwd.hpp"
#include "power/fwd.hpp"
#include "time/fwd.hpp"
#include "utils.h"

namespace experiment
{
    namespace sads
    {
        /**
         * @brief SADS experiment
         */
        class SADSExperiment final : public experiments::IExperiment
        {
          public:
            /**
             * @brief ctor.
             * @param[in] fileSystem Reference to object that provides access to filesystem services.
             * @param[in] adcsCoordinator Reference to adcs subsystem coordinator
             * @param[in] gyroDriver Reference to gyroscope driver
             * @param[in] payloadDriver Reference to payload driver
             * @param[in] powerController Reference to power controller
             * @param[in] photoService Reference to service capable of taking photos
             * @param[in] timeProvider Reference to current time provider.
             */
            SADSExperiment(services::fs::IFileSystem& fileSystem,
                ::adcs::IAdcsCoordinator& adcsCoordinator,
                devices::gyro::IGyroscopeDriver& gyroDriver,
                devices::payload::IPayloadDeviceDriver& payloadDriver,
                services::power::IPowerControl& powerController,
                services::photo::IPhotoService& photoService,
                services::time::ICurrentTime& timeProvider);

            virtual experiments::ExperimentCode Type() override;

            virtual experiments::StartResult Start() override;

            virtual experiments::IterationResult Iteration() override;

            virtual void Stop(experiments::IterationResult lastResult) override;

            /**
             * @brief Experiment code
             */
            static constexpr experiments::ExperimentCode Code = 0x0A;

            /**
             * @brief Update the SADS controller reference.
             * @param sadsController Reference to SADS controller object.
             */
            void SetSADSController(mission::IDeploySolarArray& sadsController);

            /**
             * @brief Returns time to next experiment telemetry acquisition.
             * @param time Current mission time.
             * @return Time to next experiment telemetry acquisition or default value if it is not available
             */
            std::chrono::milliseconds TimeToGetTelemetry(const Option<std::chrono::milliseconds>& time) const;

            /**
              * @brief Returns time to next experiment end.
              * @param time Current mission time.
              * @return Time to next experiment end or default value if it is not available
              */
            std::chrono::milliseconds TimeToEnd(const Option<std::chrono::milliseconds>& time) const;

            /**
             * @brief Returns information whether the telemetry should be immediatelly acquired.
             * @param time Current mission time.
             * @return True when experiment telemetry should be immediatelly acquired false otherwise.
             */
            bool NeedToGetTelemetry(const Option<std::chrono::milliseconds>& time) const;

            /**
             * @brief Returns information whether the experiment should be immediatelly ended.
             * @param time Current mission time.
             * @return True when experiment should be immediatelly ended false otherwise.
             */
            bool NeedToEnd(const Option<std::chrono::milliseconds>& time) const;

            /**
             * @brief Acquire experiment telemetry & save it to experiment file.
             * @param time Current mission time.
             */
            void GetTelemetry(const Option<std::chrono::milliseconds>& time);

            /**
             * @brief Returns time to the next important experiment event.
             * @param time Current mission time.
             * @return Time to the next important experiment event.
             */
            std::chrono::milliseconds TimeToNextEvent(const Option<std::chrono::milliseconds>& time) const;

            /**
             * @brief Perform experiment last action.
             */
            void FinalizeExperiment();

            /**
             * @brief Set experiment end time.
             * @param experimentEnd New experiment end time.
             */
            void SetExperimentEnd(std::chrono::milliseconds experimentEnd);

            /**
             * @brief Set next telemetry acquisition time.
             * @param nextTelemetryAcquisition Next telemetry acquisition time.
             */
            void SetNextTelemetryAcquisition(std::chrono::milliseconds nextTelemetryAcquisition);

          private:
            /**
              * @brief Save to file time telemetry.
              * @param time Current mission time.
              * @return Operation status. True in case of success, false otherwise.
              */
            bool Save(const Option<std::chrono::milliseconds>& time);

            /**
             * @brief Save to file gyroscope telemetry.
             * @param gyroTelemetry Reference to gyroscope telemetry to save.
             * @return Operation status. True in case of success, false otherwise.
             */
            bool Save(const devices::gyro::GyroscopeTelemetry& gyroTelemetry);

            /**
              * @brief Save to file SADS telemetry.
              * @param sadsTemperature SADS temperature.
              * @return Operation status. True in case of success, false otherwise.
              */
            bool Save(std::uint16_t sadsTemperature);

            /**
             * @brief Save photodiodes telemetry to file.
             * @param photodiodes Photodiodes measurements.
             * @return Operation status. True in case of success, false otherwise.
             */
            bool Save(devices::payload::PayloadTelemetry::Photodiodes& photodiodes);

            experiments::fs::ExperimentFile _file;

            std::chrono::milliseconds _experimentEnd;

            std::chrono::milliseconds _nextTelemetryAcquisition;

            std::uint8_t _photoNumber;

            /** @brief File system */
            services::fs::IFileSystem& _fileSystem;

            ::adcs::IAdcsCoordinator& _adcsCoordinator;

            devices::gyro::IGyroscopeDriver& _gyroDriver;

            devices::payload::IPayloadDeviceDriver& _payloadDriver;

            services::power::IPowerControl& _powerController;

            services::photo::IPhotoService& _photoService;

            mission::IDeploySolarArray* _sadsController;

            services::time::ICurrentTime& _timeProvider;
        };

        inline void SADSExperiment::SetSADSController(mission::IDeploySolarArray& sadsController)
        {
            this->_sadsController = &sadsController;
        }

        inline void SADSExperiment::SetExperimentEnd(std::chrono::milliseconds experimentEnd)
        {
            this->_experimentEnd = experimentEnd;
        }

        inline void SADSExperiment::SetNextTelemetryAcquisition(std::chrono::milliseconds nextTelemetryAcquisition)
        {
            this->_nextTelemetryAcquisition = nextTelemetryAcquisition;
        }
    }
}

#endif // LIBS_MISSION_EXPERIMENTS_SADS_HPP
