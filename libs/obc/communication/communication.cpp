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
    mission::IIdleStateController& idleStateController,
    mission::antenna::IDisableAntennaDeployment& disableAntennaDeployment,
    IHasState<SystemState>& stateContainer,
    services::fs::IFileSystem& fs,
    obc::OBCExperiments& experiments,
    program_flash::BootTable& bootTable,
    boot::BootSettings& bootSettings,
    IHasState<telemetry::TelemetryState>& telemetry,
    services::power::IPowerControl& powerControl,
    mission::IOpenSail& openSail)
    : Comm(commDriver),                                                 //
      UplinkProtocolDecoder(settings::CommSecurityCode),                //
      SupportedTelecommands(                                            //
          PingTelecommand(),                                            //
          DownloadFileTelecommand(fs),                                  //
          EnterIdleStateTelecommand(currentTime, idleStateController),  //
          RemoveFileTelecommand(fs),                                    //
          SetTimeCorrectionConfigTelecommand(stateContainer),           //
          PerformDetumblingExperiment(experiments),                     //
          AbortExperiment(experiments),                                 //
          ListFilesTelecommand(fs),                                     //
          EraseBootTableEntry(bootTable),                               //
          WriteProgramPart(bootTable),                                  //
          FinalizeProgramEntry(bootTable),                              //
          SetBootSlotsTelecommand(bootSettings),                        //
          SendBeaconTelecommand(telemetry),                             //
          StopAntennaDeployment(disableAntennaDeployment),              //
          PowerCycle(powerControl),                                     //
          SetErrorCounterConfig(fdir),                                  //
          OpenSail(openSail),                                           //
          GetErrorCountersConfigTelecommand(fdir.ErrorCounting(), fdir) //
          ),                                                            //
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
