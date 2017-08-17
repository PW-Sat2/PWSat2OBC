#ifndef LIBS_EXPERIMENTS_CAMERA_INCLUDE_EXPERIMENT_CAMERA_CAMERA_EXP_HPP_
#define LIBS_EXPERIMENTS_CAMERA_INCLUDE_EXPERIMENT_CAMERA_CAMERA_EXP_HPP_

#include "CameraExperimentController.hpp"
#include "experiments/experiments.h"
#include "fs/ExperimentFile.hpp"
#include "fs/fs.h"
#include "photo/camera_api.hpp"
#include "photo/photo_service.hpp"
#include "power/power.h"
#include "time/timer.h"

namespace experiment
{
    namespace camera
    {
        /**
         * @brief Interface for setting-up Camera Commissioning experiment parameters
         */
        struct ISetupCameraCommissioningExperiment
        {
            /**
             * @brief Sets file name for output file
             * @param fileName File name for output file
             *
             * @remark String is copied to internal buffer
             * @remark If string is longer than internal buffer size, it is trimmed to maximum size
             */
            virtual void SetOutputFile(const char* fileName) = 0;

            virtual void SetPhotosBaseFileName(const char* fileName) = 0;
        };

        /**
         * @brief Camera Commissioning experiment
         * @ingroup experiments
         */
        class CameraCommissioningExperiment final : public experiments::IExperiment, public ISetupCameraCommissioningExperiment
        {
          public:
            /** @brief Experiment code */
            static constexpr experiments::ExperimentCode Code = 4;

            /**
             * @brief Ctor
             * @param fileSystem File System provider
             * @param time Current time provider
             * @param powerControl Power Control provider
             * @param photoService Photo service
             */
            CameraCommissioningExperiment(services::fs::IFileSystem& fileSystem,
                services::time::ICurrentTime& time,
                services::power::IPowerControl& powerController,
                services::photo::IPhotoService& photoService);

            /**
             * @brief Method allowing to set name of file where data will be saved.
             * @param fileName The name of file where data will be saved.
             */
            virtual void SetOutputFile(const char* fileName) override;

            virtual void SetPhotosBaseFileName(const char* fileName) override;

            virtual experiments::ExperimentCode Type() override;
            virtual experiments::StartResult Start() override;
            virtual experiments::IterationResult Iteration() override;
            virtual void Stop(experiments::IterationResult lastResult) override;

          private:
            /** @brief Output file name. */
            static constexpr const char* DefaultFileName = "/camcom";

            experiments::IterationResult QuickCheckStep();
            experiments::IterationResult PhotoTestStep();

            void TakePhoto();

            /** @brief Time provider */
            services::time::ICurrentTime& _time;

            /** @brief File System provider */
            services::fs::IFileSystem& _fileSystem;

            /** @brief Power Control Driver */
            // services::power::IPowerControl& _powerControl;

            /** @brief Photo service */
            // services::photo::IPhotoService& _photoService;

            /** @brief Camera driver */
            // services::photo::ICamera& _camera;

            /** @brief Experiment file with results */
            experiments::fs::ExperimentFile _experimentFile;

            /** @brief Controller that does the camera experiment logic. */
            CameraExperimentController _controller;

            uint8_t _currentStep;

            char _fileName[30];
        };
    }
}

#endif /* LIBS_EXPERIMENTS_CAMERA_INCLUDE_EXPERIMENT_CAMERA_CAMERA_EXP_HPP_ */
