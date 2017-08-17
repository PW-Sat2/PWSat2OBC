#include "camera_exp.hpp"
#include <cstring>
#include <gsl/span>
#include "base/os.h"
#include "base/writer.h"
#include "logger/logger.h"

using experiments::IterationResult;
using experiments::StartResult;
using experiments::fs::ExperimentFile;
using services::fs::File;
using services::fs::FileOpen;
using services::fs::FileAccess;
using namespace std::chrono_literals;
using namespace services::photo;

namespace experiment
{
    namespace camera
    {
        CameraCommissioningExperiment::CameraCommissioningExperiment(services::fs::IFileSystem& fileSystem,
            services::time::ICurrentTime& time,
            services::power::IPowerControl& powerController,
            services::photo::IPhotoService& photoService)
            : _time(time), _fileSystem(fileSystem), _experimentFile(&_time), _controller(_experimentFile, powerController, photoService),
              _currentStep(0)
        {
            std::strncpy(_fileName, DefaultFileName, 30);
        }

        void CameraCommissioningExperiment::SetOutputFile(const char* fileName)
        {
            std::strncpy(this->_fileName, fileName, sizeof(this->_fileName));
            *std::end(this->_fileName) = '\0';
        }

        experiments::ExperimentCode CameraCommissioningExperiment::Type()
        {
            return Code;
        }

        StartResult CameraCommissioningExperiment::Start()
        {
            auto result = _experimentFile.Open(this->_fileSystem, _fileName, FileOpen::CreateAlways, FileAccess::WriteOnly);
            if (!result)
            {
                LOG(LOG_LEVEL_ERROR, "Opening experiment file failed");
                return StartResult::Failure;
            }

            return StartResult::Success;
        }

        IterationResult CameraCommissioningExperiment::Iteration()
        {
            ++_currentStep;

            switch (_currentStep)
            {
                default:
                    return IterationResult::Failure;
                case 1:
                    return this->_controller.PerformQuickCheck();
                case 2:
                    return this->_controller.PerformPhotoTest();
                case 3:
                    return IterationResult::Finished;
            }

            return IterationResult::Finished;
        }

        void CameraCommissioningExperiment::Stop(IterationResult /*lastResult*/)
        {
            _experimentFile.Close();
        }

        // -------------------------------------------------------------------------------------------

        CameraExperimentController::CameraExperimentController(experiments::fs::ExperimentFile& file,
            services::power::IPowerControl& powerControl,
            services::photo::IPhotoService& photoService)
            : _powerControl(powerControl), _photoService(photoService), _experimentFile(file), _fileName(DefaultBaseFileName)
        {
        }

        void CameraExperimentController::CameraQuickCheckStep(Camera camera)
        {
            _photoService.EnableCamera(camera);
            _photoService.DisableCamera(camera);
            _photoService.WaitForFinish(5min);
            auto syncResult = _photoService.GetLastSyncResult(camera);
            uint8_t stepResult = syncResult.GetIsSuccessful() ? static_cast<uint8_t>(syncResult.GetRetryCount()) : 0xFF;
            _experimentFile.Write(ExperimentFile::PID::CameraSyncCount, gsl::make_span(&stepResult, 1));
        }

        IterationResult CameraExperimentController::PerformQuickCheck()
        {
            for (int i = 0; i < 10; ++i)
            {
                CameraQuickCheckStep(Camera::Nadir);
            }

            for (int i = 0; i < 10; ++i)
            {
                CameraQuickCheckStep(Camera::Wing);
            }

            return IterationResult::LoopImmediately;
        }

        void CameraExperimentController::TakePhoto(Camera camera, PhotoResolution resolution, uint8_t buffer_number)
        {
            this->_photoService.TakePhoto(camera, resolution);
            this->_photoService.DownloadPhoto(camera, buffer_number);
        }

        IterationResult CameraExperimentController::PerformPhotoTest()
        {
            _photoService.EnableCamera(Camera::Nadir);
            _photoService.EnableCamera(Camera::Wing);

            TakePhoto(Camera::Nadir, PhotoResolution::p128, 0);
            TakePhoto(Camera::Nadir, PhotoResolution::p240, 1);
            TakePhoto(Camera::Nadir, PhotoResolution::p480, 2);

            TakePhoto(Camera::Wing, PhotoResolution::p128, 3);
            TakePhoto(Camera::Wing, PhotoResolution::p240, 4);
            TakePhoto(Camera::Wing, PhotoResolution::p480, 5);

            for (int i = 0; i < 6; ++i)
            {
                _photoService.SavePhoto(i, "%s_%d.jpg", _fileName, i);
            }

            _photoService.DisableCamera(Camera::Nadir);
            _photoService.DisableCamera(Camera::Wing);

            return IterationResult::LoopImmediately;
        }
    }
}
