#include "experiments.hpp"

namespace obc
{
    OBCExperiments::OBCExperiments(services::fs::IFileSystem& fs, adcs::IAdcsCoordinator& adcs, services::time::TimeProvider& time)
        : Experiments(                                           //
              experiment::fibo::FibonacciExperiment(fs),         //
              experiment::adcs::DetumblingExperiment(adcs, time) //
              )
    {
    }

    void OBCExperiments::InitializeRunlevel1()
    {
        this->ExperimentsController.SetExperiments(this->Experiments.All());
        this->ExperimentsController.Initialize();
    }
}
