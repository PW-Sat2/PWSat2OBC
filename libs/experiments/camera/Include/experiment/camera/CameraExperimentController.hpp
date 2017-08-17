#ifndef LIBS_EXPERIMENTS_CAMERA_INCLUDE_EXPERIMENT_CAMERA_CAMERAEXPERIMENTCONTROLLER_HPP_
#define LIBS_EXPERIMENTS_CAMERA_INCLUDE_EXPERIMENT_CAMERA_CAMERAEXPERIMENTCONTROLLER_HPP_

#include "experiments/experiments.h"
#include "fs/ExperimentFile.hpp"
#include "photo/camera_api.hpp"
#include "photo/photo_service.hpp"
#include "power/power.h"

namespace experiment
{
    namespace camera
    {
        class CameraExperimentController
        {
          public:
            CameraExperimentController(experiments::fs::ExperimentFile& file,
                services::power::IPowerControl& powerControl,
                services::photo::IPhotoService& photoService);

            experiments::IterationResult PerformQuickCheck();
            experiments::IterationResult PerformPhotoTest();
            void SetPhotoFilesBaseName(const char* fileName);

          private:
            /** @brief Output file name. */
            static constexpr const char* DefaultBaseFileName = "/camcom";

            void TakePhoto(services::photo::Camera camera, services::photo::PhotoResolution resolution, uint8_t buffer_number);
            void CameraQuickCheckStep(services::photo::Camera camera);

            /** @brief Power Control Driver */
            services::power::IPowerControl& _powerControl;

            /** @brief Photo service */
            services::photo::IPhotoService& _photoService;

            /** @brief Experiment file with results */
            experiments::fs::ExperimentFile& _experimentFile;

            /** @brief Base file name for photos generated during processing. */
            const char* _fileName;
        };
    }
}

#endif /* LIBS_EXPERIMENTS_CAMERA_INCLUDE_EXPERIMENT_CAMERA_CAMERAEXPERIMENTCONTROLLER_HPP_ */
