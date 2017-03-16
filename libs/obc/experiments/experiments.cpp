#include "experiments.hpp"

namespace obc
{
    OBCExperiments::OBCExperiments(services::fs::IFileSystem& fs, adcs::IAdcsCoordinator& adcs, services::time::TimeProvider& time)
        :           //
          Fibo(fs), //
          Detumbling(adcs, time),
          Experiments{&Fibo, &Detumbling}
    {
    }

    void OBCExperiments::Initialize()
    {
        this->ExperimentsController.SetExperiments(this->Experiments);
        this->ExperimentsController.Initialize();
    }
}
