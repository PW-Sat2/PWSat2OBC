#include "experiments.hpp"

namespace obc
{
    OBCExperiments::OBCExperiments(services::fs::IFileSystem& fs,
        adcs::IAdcsCoordinator& adcs,
        services::time::TimeProvider& time,
        devices::gyro::IGyroscopeDriver& gyro,
        services::power::IPowerControl& powerControl,
        devices::suns::ISunSDriver& suns,
        devices::payload::IPayloadDeviceDriver& payload,
        devices::n25q::RedundantN25QDriver& n25q,
        devices::comm::ITransmitter& transmitter)
        : Experiments(                                                                       //
              experiment::fibo::FibonacciExperiment(fs),                                     //
              experiment::adcs::DetumblingExperiment(adcs, time),                            //
              experiment::leop::LaunchAndEarlyOrbitPhaseExperiment(gyro, time, fs),          //
              experiment::suns::SunSExperiment(powerControl, time, suns, payload, gyro, fs), //
              experiment::erase_flash::EraseFlashExperiment(n25q, transmitter)               //
              )
    {
    }

    void OBCExperiments::InitializeRunlevel1()
    {
        this->ExperimentsController.SetExperiments(this->Experiments.All());
        this->ExperimentsController.Initialize();
    }
}
