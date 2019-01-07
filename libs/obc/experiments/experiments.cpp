#include "experiments.hpp"

namespace obc
{
    OBCExperiments::OBCExperiments(services::fs::IFileSystem& fs,
        services::time::TimeProvider& time,
        devices::gyro::IGyroscopeDriver& gyro,
        services::power::IPowerControl& powerControl,
        devices::suns::ISunSDriver& suns,
        devices::payload::IPayloadDeviceDriver& payload,
        devices::n25q::RedundantN25QDriver& n25q,
        devices::comm::ITransmitter& transmitter,
        services::photo::IPhotoService& photoService,
        devices::eps::IEpsTelemetryProvider& epsProvider,
        error_counter::IErrorCountingTelemetryProvider* errorCounterProvider,
        temp::ITemperatureReader* temperatureProvider,
        program_flash::BootTable& bootTable,
        program_flash::IFlashDriver& programFlashDriver)
        : Experiments(                                                                       //
              experiment::fibo::FibonacciExperiment(fs),                                     //
              experiment::suns::SunSExperiment(powerControl, time, suns, payload, gyro, fs), //
              experiment::erase_flash::EraseFlashExperiment(n25q, transmitter),              //
              experiment::radfet::RadFETExperiment(fs, payload, powerControl, time),         //
              experiment::payload::PayloadCommissioningExperiment(payload,
                  fs,
                  powerControl,
                  time,
                  suns,
                  photoService,
                  epsProvider,
                  errorCounterProvider,
                  temperatureProvider,
                  &ExperimentsController),
              experiment::camera::CameraCommissioningExperiment(fs, time, photoService),
              experiment::program::CopyBootSlotsExperiment(bootTable, programFlashDriver, transmitter))
    {
    }

    void OBCExperiments::InitializeRunlevel1()
    {
        this->ExperimentsController.SetExperiments(this->Experiments.All());
        this->ExperimentsController.Initialize();
    }
}
