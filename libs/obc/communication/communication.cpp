#include "communication.h"
#include "gsl/span"
#include "logger/logger.h"
#include "settings.h"
#include "telecommunication/downlink.h"

using std::uint8_t;
using gsl::span;
using drivers::i2c::II2CBus;
using telecommunication::uplink::IHandleTeleCommand;

using namespace obc;
using namespace obc::telecommands;

OBCCommunication::OBCCommunication(obc::FDIR& fdir,
    devices::comm::CommObject& commDriver,
    services::time::ICurrentTime& currentTime,
    devices::rtc::IRTC& rtc,
    mission::IIdleStateController& idleStateController,
    mission::antenna::IDisableAntennaDeployment& disableAntennaDeployment,
    IHasState<SystemState>& stateContainer,
    services::fs::IFileSystem& fs,
    obc::OBCExperiments& experiments,
    program_flash::BootTable& bootTable,
    boot::BootSettings& bootSettings,
    IHasState<telemetry::TelemetryState>& telemetry,
    services::power::IPowerControl& powerControl,
    mission::IOpenSail& openSail,
    mission::IDeploySolarArray& deploySolarArray,
    mission::ITimeSynchronization& timeSynchronization,
    drivers::i2c::II2CBus& systemBus,
    drivers::i2c::II2CBus& payload,
    devices::suns::ISunSDriver& experimentalSunS,
    devices::payload::IPayloadDeviceDriver& payloadDriver,
    devices::gyro::IGyroscopeDriver& gyro,
    services::photo::IPhotoService& photo,
    mission::ITriggerPeriodicMessage& triggerPeriodicMessage)
    : Comm(commDriver),                                                                                                               //
      UplinkProtocolDecoder(settings::CommSecurityCode),                                                                              //
      SupportedTelecommands(                                                                                                          //
          PingTelecommand(),                                                                                                          //
          DownloadFileTelecommand(fs),                                                                                                //
          EnterIdleStateTelecommand(currentTime, idleStateController),                                                                //
          RemoveFileTelecommand(fs),                                                                                                  //
          SetTimeCorrectionConfigTelecommand(stateContainer),                                                                         //
          SetTimeTelecommand(stateContainer, currentTime, rtc, timeSynchronization),                                                  //
          PerformDetumblingExperiment(                                                                                                //
              experiments.ExperimentsController,                                                                                      //
              experiments.Get<experiment::adcs::DetumblingExperiment>()                                                               //
              ),                                                                                                                      //
          AbortExperiment(experiments.ExperimentsController),                                                                         //
          ListFilesTelecommand(fs),                                                                                                   //
          EraseBootTableEntry(bootTable),                                                                                             //
          WriteProgramPart(bootTable),                                                                                                //
          FinalizeProgramEntry(bootTable),                                                                                            //
          SetBootSlotsTelecommand(bootSettings),                                                                                      //
          SendBeaconTelecommand(telemetry),                                                                                           //
          StopAntennaDeployment(disableAntennaDeployment),                                                                            //
          PowerCycle(powerControl),                                                                                                   //
          SetErrorCounterConfig(fdir),                                                                                                //
          OpenSail(openSail),                                                                                                         //
          GetErrorCountersConfigTelecommand(fdir.ErrorCounting(), fdir),                                                              //
          SetPeriodicMessageTelecommand(stateContainer),                                                                              //
          PerformSunSExperiment(experiments.ExperimentsController, experiments.Get<experiment::suns::SunSExperiment>()),              //
          EraseFlashTelecommand(experiments.ExperimentsController, experiments.Get<experiment::erase_flash::EraseFlashExperiment>()), //
          RawI2CTelecommand(systemBus, payload),                                                                                      //
          PerformRadFETExperiment(experiments.ExperimentsController, experiments.Get<experiment::radfet::RadFETExperiment>()),        //
          GetSunSDataSetsTelecommand(powerControl, currentTime, experimentalSunS, payloadDriver, gyro),                               //
          PerformSailExperiment(experiments.ExperimentsController),                                                                   //
          TakePhoto(photo),                                                                                                           //
          DeploySolarArray(deploySolarArray),                                                                                         //
          PerformPayloadCommisioningExperiment(
              experiments.ExperimentsController, experiments.Get<experiment::payload::PayloadCommissioningExperiment>()), //
          GetPersistentStateTelecommand(stateContainer),                                                                  //
          PurgePhoto(photo),                                                                                              //
          SendPeriodicMessageTelecommand(triggerPeriodicMessage)                                                          //
          ),                                                                                                              //
      TelecommandHandler(UplinkProtocolDecoder, SupportedTelecommands.Get())
{
}

void OBCCommunication::InitializeRunlevel1()
{
    this->Comm.SetFrameHandler(this->TelecommandHandler);
}

void OBCCommunication::InitializeRunlevel2()
{
    if (!this->Comm.Restart())
    {
        LOG(LOG_LEVEL_ERROR, "Unable to restart comm");
    }
}
