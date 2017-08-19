#include "CameraExperimentController.hpp"
#include "base/writer.h"
#include "photo/photo_service.hpp"

using experiments::IterationResult;
using experiments::fs::ExperimentFile;
using namespace std::chrono_literals;
using namespace services::photo;

namespace experiment
{
    namespace camera
    {
        CameraExperimentController::CameraExperimentController(
            experiments::fs::ExperimentFile& file, services::photo::IPhotoService& photoService)
            : _photoService(photoService), _experimentFile(file), _fileName(DefaultBaseFileName)
        {
        }

        void CameraExperimentController::SetPhotoFilesBaseName(const char* fileName)
        {
            _fileName = fileName;
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
            this->_photoService.EnableCamera(Camera::Nadir);
            this->_photoService.EnableCamera(Camera::Wing);

            TakePhoto(Camera::Nadir, PhotoResolution::p128, 0);
            TakePhoto(Camera::Nadir, PhotoResolution::p240, 1);
            TakePhoto(Camera::Nadir, PhotoResolution::p480, 2);

            TakePhoto(Camera::Wing, PhotoResolution::p128, 3);
            TakePhoto(Camera::Wing, PhotoResolution::p240, 4);
            TakePhoto(Camera::Wing, PhotoResolution::p480, 5);

            this->_photoService.DisableCamera(Camera::Nadir);
            this->_photoService.DisableCamera(Camera::Wing);
            this->_photoService.WaitForFinish(InfiniteTimeout);

            for (int i = 0; i < 6; ++i)
            {
                if (!this->_photoService.IsEmpty(i))
                {
                    this->_photoService.SavePhoto(i, "%s_%d.jpg", _fileName, i);
                }
            }

            this->_photoService.Reset();
            this->_photoService.WaitForFinish(InfiniteTimeout);

            return IterationResult::LoopImmediately;
        }
    }
}
