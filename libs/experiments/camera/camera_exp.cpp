#include "camera_exp.hpp"
#include "logger/logger.h"
#include "photo/photo_service.hpp"
#include "utils.h"

using experiments::IterationResult;
using experiments::StartResult;
using experiments::fs::ExperimentFile;
using services::fs::File;
using services::fs::FileOpen;
using services::fs::FileAccess;

namespace experiment
{
    namespace camera
    {
        CameraCommissioningExperiment::CameraCommissioningExperiment(
            services::fs::IFileSystem& fileSystem, services::time::ICurrentTime& time, services::photo::IPhotoService& photoService)
            : _time(time), _fileSystem(fileSystem), _experimentFile(&_time), _photoService(photoService),
              _controller(_experimentFile, photoService), _currentStep(0)
        {
            strsafecpy(_fileName, DefaultFileName, 30);
        }

        CameraCommissioningExperiment::CameraCommissioningExperiment(CameraCommissioningExperiment&& other)
            : _time(other._time), _fileSystem(other._fileSystem), _experimentFile(std::move(other._experimentFile)),
              _photoService(other._photoService), _controller(_experimentFile, _photoService), _currentStep(other._currentStep)
        {
            strsafecpy(_fileName, other._fileName, count_of(other._fileName));
        }

        void CameraCommissioningExperiment::SetOutputFilesBaseName(gsl::cstring_span<> fileName)
        {
            strsafecpy(this->_fileName, fileName);
            _controller.SetPhotoFilesBaseName(this->_fileName);
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

            _currentStep = 0;
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
    }
}
