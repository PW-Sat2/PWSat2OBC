#include "experiments.hpp"

namespace obc
{
    OBCExperiments::OBCExperiments(services::fs::IFileSystem& fs,
        adcs::IAdcsCoordinator& adcs,
        services::time::TimeProvider& time,
        devices::gyro::IGyroscopeDriver& gyro)
        :                         //
          Fibo(fs),               //
          Detumbling(adcs, time), //
          LEOP(gyro, time, fs),   //
          Experiments{&Fibo, &Detumbling, &LEOP}
    {
    }

    void OBCExperiments::InitializeRunlevel1()
    {
        this->ExperimentsController.SetExperiments(this->Experiments);
        this->ExperimentsController.Initialize();
    }
}
