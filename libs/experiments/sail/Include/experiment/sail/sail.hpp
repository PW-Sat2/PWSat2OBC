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
             * @brief Ctor
             * @param fileSystem File system
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

            std::chrono::milliseconds TimeToGetTelemetry(const Option<std::chrono::milliseconds>& time) const;

            std::chrono::milliseconds TimeToTakePhoto(const Option<std::chrono::milliseconds>& time) const;

            std::chrono::milliseconds TimeToEnd(const Option<std::chrono::milliseconds>& time) const;

            bool NeedToGetTelemetry(const Option<std::chrono::milliseconds>& time) const;

            bool NeedToTakePhoto(const Option<std::chrono::milliseconds>& time) const;

            bool NeedToEnd(const Option<std::chrono::milliseconds>& time) const;

            void GetTelemetry(const Option<std::chrono::milliseconds>& time);

            void TakePhoto(const Option<std::chrono::milliseconds>& time);

            std::chrono::milliseconds TimeToNextEvent(const Option<std::chrono::milliseconds>& time) const;

            services::photo::Camera GetNextCamera() const;

          private:
            bool Save(const devices::gyro::GyroscopeTelemetry& gyroTelemetry);

            bool Save(bool sailIndicator, std::uint16_t sailTemperature);

            experiments::fs::ExperimentFile _file;

            Option<std::chrono::milliseconds> _experimentBegin;

            Option<std::chrono::milliseconds> _lastTelemetryAcquisition;

            Option<std::chrono::milliseconds> _lastPhotoTaken;

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
    }
}

#endif
