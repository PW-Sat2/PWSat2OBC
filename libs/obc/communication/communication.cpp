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
    II2CBus& systemBus,
    services::time::ICurrentTime& currentTime,
    mission::IIdleStateController& idleStateController,
    services::fs::IFileSystem& fs,
    obc::OBCExperiments& experiments,
    program_flash::BootTable& bootTable)
    : UplinkProtocolDecoder(settings::CommSecurityCode),                      //
      SupportedTelecommands(                                                  //
          PingTelecommand(),                                                  //
          DownloadFileTelecommand(fs),                                        //
          EnterIdleStateTelecommand(currentTime, idleStateController),        //
          RemoveFileTelecommand(fs),                                          //
          PerformDetumblingExperiment(experiments),                           //
          AbortExperiment(experiments),                                       //
          ListFilesTelecommand(fs),                                           //
          EraseBootTableEntry(bootTable),                                     //
          WriteProgramPart(bootTable),                                        //
          FinalizeProgramEntry(bootTable)                                     //
          ),                                                                  //
      TelecommandHandler(UplinkProtocolDecoder, SupportedTelecommands.Get()), //
      CommDriver(fdir.ErrorCounting(), systemBus, TelecommandHandler)
{
}

void OBCCommunication::InitializeRunlevel1()
{
    this->CommDriver.Initialize();
}

void OBCCommunication::InitializeRunlevel2()
{
    if (!this->CommDriver.Restart())
    {
        LOG(LOG_LEVEL_ERROR, "Unable to restart comm");
    }
    this->CommDriver.Resume();
}
