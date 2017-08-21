#ifndef LIBS_EXPERIMENTS_CAMERA_INCLUDE_EXPERIMENT_CAMERA_CAMERAEXPERIMENTCONTROLLER_HPP_
#define LIBS_EXPERIMENTS_CAMERA_INCLUDE_EXPERIMENT_CAMERA_CAMERAEXPERIMENTCONTROLLER_HPP_

#include "experiments/experiments.h"
#include "fs/ExperimentFile.hpp"
#include "photo/fwd.hpp"
#include "utils.h"

namespace experiment
{
    namespace camera
    {
        /**
         * @brief Class that does Camera Commisioning Experiment logic.
         * It is created to two allow two experiments (Payload and Camera) could execute the same code.
         */
        class CameraExperimentController : private NotCopyable, private NotMoveable
        {
          public:
            /**
             * @brief Constructor
             * @param file The experiment file used by parent experiment
             * @param photoService The Photo Service.
             */
            CameraExperimentController(experiments::fs::ExperimentFile& file, services::photo::IPhotoService& photoService);

            /**
             * @brief Performs quick check
             * @returns Iteration result.
             *  */
            experiments::IterationResult PerformQuickCheck();

            /**
             * @brief Performs photo test
             * @returns Iteration result.
             *  */
            experiments::IterationResult PerformPhotoTest();

            /**
             * @brief Sets base filename for photos generated in Photo Test
             * @param fileName The base file name.
             *  */
            void SetPhotoFilesBaseName(const char* fileName);

          private:
            /** @brief Output file name. */
            static constexpr const char* DefaultBaseFileName = "/camcom";

            void TakePhoto(services::photo::Camera camera, services::photo::PhotoResolution resolution, uint8_t buffer_number);
            void CameraQuickCheckStep(services::photo::Camera camera);

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
