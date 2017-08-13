#ifndef LIBS_MISSION_EXPERIMENTS_SAIL_HPP
#define LIBS_MISSION_EXPERIMENTS_SAIL_HPP

#pragma once

#include <cstdint>
#include "adcs/adcs.hpp"
#include "experiments/experiments.h"
#include "fs/ExperimentFile.hpp"
#include "fs/fs.h"
#include "gpio/forward.h"
#include "gyro/fwd.hpp"
#include "mission/sail_fwd.hpp"
#include "payload/interfaces.h"
#include "photo/fwd.hpp"
#include "power/fwd.hpp"
#include "time/fwd.hpp"
#include "utils.h"

namespace experiment
{
    namespace sail
    {
        /**
         * @brief Sail experiment
         */
        class SailExperiment final : public experiments::IExperiment
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
             * @param[in] sailState Reference to pin connected to sail indicator.
             * @param[in] timeProvider Reference to current time provider.
             */
            SailExperiment(services::fs::IFileSystem& fileSystem,
                ::adcs::IAdcsCoordinator& adcsCoordinator,
                devices::gyro::IGyroscopeDriver& gyroDriver,
                devices::payload::IPayloadDeviceDriver& payloadDriver,
                services::power::IPowerControl& powerController,
                services::photo::IPhotoService& photoService,
                const drivers::gpio::Pin& sailState,
                services::time::ICurrentTime& timeProvider);

            virtual experiments::ExperimentCode Type() override;

            virtual experiments::StartResult Start() override;

            virtual experiments::IterationResult Iteration() override;

            virtual void Stop(experiments::IterationResult lastResult) override;

            /**
             * @brief Experiment code
             */
            static constexpr experiments::ExperimentCode Code = 0x07;

            /**
             * @brief Update the sail controller reference.
             * @param sailController Reference to sail controller object.
             */
            void SetSailController(mission::IOpenSail& sailController);

            /**
             * @brief Returns time to next experiment telemetry acquisition.
             * @param time Current mission time.
             * @return Time to next experiment telemetry acquisition or default value if it is not available
             */
            std::chrono::milliseconds TimeToGetTelemetry(const Option<std::chrono::milliseconds>& time) const;

            /**
             * @brief Returns time to next experiment photo.
             * @param time Current mission time.
             * @return Time to next experiment photo or default value if it is not available
             */
            std::chrono::milliseconds TimeToTakePhoto(const Option<std::chrono::milliseconds>& time) const;

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
             * @brief Returns information whether the photo should be immediatelly taken.
             * @param time Current mission time.
             * @return True when photo should be immediatelly taken false otherwise.
             */
            bool NeedToTakePhoto(const Option<std::chrono::milliseconds>& time) const;

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
             * @brief Take photo on current camera & switch to the next one.
             * @param time Current mission time.
             */
            void TakePhoto(const Option<std::chrono::milliseconds>& time);

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
             * @brief Save all cached photos to experiment files.
             */
            void SavePhotos();

            /**
             * @brief Set sail experiment end time.
             * @param experimentEnd New experiment end time.
             */
            void SetExperimentEnd(std::chrono::milliseconds experimentEnd);

            /**
             * @brief Set next telemetry acquisition time.
             * @param nextTelemetryAcquisition Next telemetry acquisition time.
             */
            void SetNextTelemetryAcquisition(std::chrono::milliseconds nextTelemetryAcquisition);

            /**
             * @brief Sets next photo time.
             * @param nextPhotoTaken Next photo time.
             */
            void SetNextPhotoTaken(std::chrono::milliseconds nextPhotoTaken);

          private:
            /**
             * @brief Make a photo.
             * @param camera Id of the camera to take photo
             * @param resolution Requested photo resolution.
             */
            void TakePhoto(services::photo::Camera camera, services::photo::PhotoResolution resolution);

            /**
             * @brief Returns id of the camera that should be used next time.
             * @return Id of the next camera.
             */
            services::photo::Camera GetNextCamera() const;

            /**
              * @brief Save to file sail telemetry.
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
              * @brief Save to file sail telemetry.
              * @param sailIndicator Sail opening indicator.
              * @param sailTemperature Sail temperature.
              * @return Operation status. True in case of success, false otherwise.
              */
            bool Save(bool sailIndicator, std::uint16_t sailTemperature);

            experiments::fs::ExperimentFile _file;

            std::chrono::milliseconds _experimentEnd;

            std::chrono::milliseconds _nextTelemetryAcquisition;

            std::chrono::milliseconds _nextPhoto;

            std::uint8_t _photoNumber;

            services::photo::Camera _lastCamera;

            /** @brief File system */
            services::fs::IFileSystem& _fileSystem;

            ::adcs::IAdcsCoordinator& _adcsCoordinator;

            devices::gyro::IGyroscopeDriver& _gyroDriver;

            devices::payload::IPayloadDeviceDriver& _payloadDriver;

            services::power::IPowerControl& _powerController;

            services::photo::IPhotoService& _photoService;

            mission::IOpenSail* _sailController;

            services::time::ICurrentTime& _timeProvider;

            const drivers::gpio::Pin& _sailState;
        };

        inline void SailExperiment::SetSailController(mission::IOpenSail& sailController)
        {
            this->_sailController = &sailController;
        }

        inline services::photo::Camera SailExperiment::GetNextCamera() const
        {
            return (this->_lastCamera == services::photo::Camera::Wing) ? services::photo::Camera::Nadir : services::photo::Camera::Wing;
        }

        inline void SailExperiment::SetExperimentEnd(std::chrono::milliseconds experimentEnd)
        {
            this->_experimentEnd = experimentEnd;
        }

        inline void SailExperiment::SetNextTelemetryAcquisition(std::chrono::milliseconds nextTelemetryAcquisition)
        {
            this->_nextTelemetryAcquisition = nextTelemetryAcquisition;
        }

        inline void SailExperiment::SetNextPhotoTaken(std::chrono::milliseconds nextPhotoTaken)
        {
            this->_nextPhoto = nextPhotoTaken;
        }
    }
}

#endif
