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
        devices::comm::ITransmitter& transmitter,
        services::photo::IPhotoService& photoService,
        const drivers::gpio::Pin& sailState,
        telemetry::IImtqDataProvider& imtq,
        devices::eps::IEpsTelemetryProvider& epsProvider,
        error_counter::IErrorCountingTelemetryProvider* errorCounterProvider,
        temp::ITemperatureReader* temperatureProvider)
        : Experiments(                                                                                                //
              experiment::fibo::FibonacciExperiment(fs),                                                              //
              experiment::adcs::DetumblingExperiment(adcs, time, powerControl, gyro, payload, imtq, fs),              //
              experiment::leop::LaunchAndEarlyOrbitPhaseExperiment(gyro, time, fs),                                   //
              experiment::suns::SunSExperiment(powerControl, time, suns, payload, gyro, fs),                          //
              experiment::erase_flash::EraseFlashExperiment(n25q, transmitter),                                       //
              experiment::radfet::RadFETExperiment(fs, payload, powerControl, time),                                  //
              experiment::sail::SailExperiment(fs, adcs, gyro, payload, powerControl, photoService, sailState, time), //
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
              experiment::sads::SADSExperiment(fs, adcs, gyro, payload, powerControl, photoService, time),
              experiment::camera::CameraCommissioningExperiment(fs, time, photoService))
    {
    }

    void OBCExperiments::InitializeRunlevel1()
    {
        this->ExperimentsController.SetExperiments(this->Experiments.All());
        this->ExperimentsController.Initialize();
    }
}
