#include "camera_exp.hpp"
#include "logger/logger.h"
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
            : _time(time), _fileSystem(fileSystem), _experimentFile(&_time), _controller(_experimentFile, photoService), _currentStep(0)
        {
            strsafecpy(_fileName, DefaultFileName, 30);
        }

        void CameraCommissioningExperiment::SetOutputFilesBaseName(gsl::cstring_span<> fileName)
        {
            strsafecpy(this->_fileName, fileName);
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
    }
}
