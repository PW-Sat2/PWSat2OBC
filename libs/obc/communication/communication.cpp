#include "communication.h"
#include "gsl/span"
#include "logger/logger.h"
#include "settings.h"
#include "telecommunication/downlink.h"

using drivers::i2c::II2CBus;
using gsl::span;
using std::uint8_t;
using telecommunication::uplink::IHandleTeleCommand;

using namespace obc;
using namespace obc::telecommands;

OBCCommunication::OBCCommunication(obc::FDIR& fdir,
    devices::comm::CommObject& commDriver,
    services::time::ICurrentTime& currentTime,
    devices::rtc::IRTC& rtc,
    mission::IIdleStateController& idleStateController,
    IHasState<SystemState>& stateContainer,
    services::fs::IFileSystem& fs,
    obc::OBCExperiments& experiments,
    program_flash::BootTable& bootTable,
    boot::BootSettings& bootSettings,
    IHasState<telemetry::TelemetryState>& telemetry,
    services::power::IPowerControl& powerControl,
    mission::ITimeSynchronization& timeSynchronization,
    drivers::i2c::II2CBus& systemBus,
    drivers::i2c::II2CBus& payload,
    devices::suns::ISunSDriver& experimentalSunS,
    devices::payload::IPayloadDeviceDriver& payloadDriver,
    devices::gyro::IGyroscopeDriver& gyro,
    services::photo::IPhotoService& photo,
    devices::eps::IEPSDriver& epsDriver,
    adcs::IAdcsCoordinator& adcsCoordinator)
    : Comm(commDriver),                                                              //
      UplinkProtocolDecoder(settings::CommSecurityCode),                             //
      SupportedTelecommands(                                                         //
          PingTelecommand(),                                                         //
          DownloadFileTelecommand(fs),                                               //
          EnterIdleStateTelecommand(currentTime, idleStateController),               //
          RemoveFileTelecommand(fs),                                                 //
          SetTimeCorrectionConfigTelecommand(stateContainer),                        //
          SetTimeTelecommand(stateContainer, currentTime, rtc, timeSynchronization), //
          AbortExperiment(experiments.ExperimentsController),                        //
          ListFilesTelecommand(fs),                                                  //
          EraseBootTableEntry(bootTable),                                            //
          WriteProgramPart(bootTable),                                               //
          FinalizeProgramEntry(bootTable),                                           //
          SetBootSlotsTelecommand(bootSettings),                                     //
          SendBeaconTelecommand(telemetry),                                          //
          SetAntennaDeploymentMaskTelecommand(stateContainer),                       //
          PowerCycle(powerControl),                                                  //
          SetErrorCounterConfig(
              fdir), //                                                                                                    //
          GetErrorCountersConfigTelecommand(fdir.ErrorCounting(), fdir),                                                              //
          SetPeriodicMessageTelecommand(stateContainer),                                                                              //
          PerformSunSExperiment(experiments.ExperimentsController, experiments.Get<experiment::suns::SunSExperiment>()),              //
          EraseFlashTelecommand(experiments.ExperimentsController, experiments.Get<experiment::erase_flash::EraseFlashExperiment>()), //
          RawI2CTelecommand(systemBus, payload),                                                                                      //
          PerformRadFETExperiment(experiments.ExperimentsController, experiments.Get<experiment::radfet::RadFETExperiment>()),        //
          GetSunSDataSetsTelecommand(powerControl, currentTime, experimentalSunS, payloadDriver, gyro),                               //
          TakePhoto(photo),                                                                                                           //
          PerformPayloadCommisioningExperiment(
              experiments.ExperimentsController, experiments.Get<experiment::payload::PayloadCommissioningExperiment>()), //
          GetPersistentStateTelecommand(stateContainer),                                                                  //
          PurgePhoto(photo),                                                                                              //
          PerformCameraCommisioningExperiment(
              experiments.ExperimentsController, experiments.Get<experiment::camera::CameraCommissioningExperiment>()), //
          SendPeriodicMessageTelecommand(stateContainer),                                                               //
          CompileInfoTelecommand(),                                                                                     //
          ResetTransmitterTelecommand(),                                                                                //
          DisableOverheatSubmodeTelecommand(epsDriver),                                                                 //
          SetBitrateTelecommand(),                                                                                      //
          PerformCopyBootSlotsExperiment(
              experiments.ExperimentsController, experiments.Get<experiment::program::CopyBootSlotsExperiment>()), //
          SetBuiltinDetumblingBlockMaskTelecommand(stateContainer, adcsCoordinator),                               //
          SetAdcsModeTelecommand(adcsCoordinator),                                                                 //
          obc::telecommands::ReadMemoryTelecommand()                                                               //
          ),                                                                                                       //
      TelecommandHandler(UplinkProtocolDecoder, SupportedTelecommands.Get())
{
}

void OBCCommunication::InitializeRunlevel1()
{
    this->Comm.SetFrameHandler(this->TelecommandHandler);
    if (!this->Comm.RestartHardware())
    {
        LOG(LOG_LEVEL_ERROR, "Unable to restart COMM hardware");
    }
}

void OBCCommunication::InitializeRunlevel2()
{
    if (!this->Comm.StartTask())
    {
        LOG(LOG_LEVEL_ERROR, "Unable to start comm task");
    }
}
