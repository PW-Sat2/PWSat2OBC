#include "experiments.hpp"

namespace obc
{
    OBCExperiments::OBCExperiments(services::fs::IFileSystem& fs)
        :           //
          Fibo(fs), //
          Experiments{&Fibo}
    {
    }

    void OBCExperiments::Initialize()
    {
        this->ExperimentsController.SetExperiments(this->Experiments);
        this->ExperimentsController.Initialize();
    }
}
