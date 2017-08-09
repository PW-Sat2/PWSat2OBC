#include "sail.hpp"
#include "base/writer.h"
#include "logger/logger.h"

using services::fs::File;
using services::fs::IFileSystem;
using services::fs::FileOpen;
using services::fs::FileAccess;

namespace experiment
{
    namespace sail
    {
        SailExperiment::SailExperiment(IFileSystem& fileSystem) : _fileSystem(fileSystem)
        {
        }

        experiments::ExperimentCode SailExperiment::Type()
        {
            return Code;
        }

        experiments::StartResult SailExperiment::Start()
        {
            return experiments::StartResult::Success;
        }

        experiments::IterationResult SailExperiment::Iteration()
        {
            return experiments::IterationResult::Finished;
        }

        void SailExperiment::Stop(experiments::IterationResult lastResult)
        {
            UNREFERENCED_PARAMETER(lastResult);
        }
    }
}
