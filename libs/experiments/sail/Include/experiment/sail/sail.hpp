#ifndef LIBS_MISSION_EXPERIMENTS_SAIL_HPP
#define LIBS_MISSION_EXPERIMENTS_SAIL_HPP

#include <cstdint>
#include "adcs/adcs.hpp"
#include "experiments/experiments.h"
#include "fs/fs.h"
#include "gpio/forward.h"
#include "gyro/fwd.hpp"
#include "payload/interfaces.h"
#include "photo/fwd.hpp"
#include "power/fwd.hpp"
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
                const drivers::gpio::Pin& sailState);

            virtual experiments::ExperimentCode Type() override;

            virtual experiments::StartResult Start() override;

            virtual experiments::IterationResult Iteration() override;

            virtual void Stop(experiments::IterationResult lastResult) override;

            /**
             * @brief Experiment code
             */
            static constexpr experiments::ExperimentCode Code = 0x07;

          private:
            /** @brief File system */
            services::fs::IFileSystem& _fileSystem;

            ::adcs::IAdcsCoordinator& _adcsCoordinator;

            devices::gyro::IGyroscopeDriver& _gyroDriver;

            devices::payload::IPayloadDeviceDriver& _payloadDriver;

            services::power::IPowerControl& _powerController;

            services::photo::IPhotoService& _photoService;

            const drivers::gpio::Pin& _sailState;
        };
    }
}

#endif
