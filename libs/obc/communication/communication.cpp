#include "communication.h"
#include "gsl/span"
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
          EraseBootTableEntry(bootTable),                                     //
          WriteProgramPart(bootTable),                                        //
          FinalizeProgramEntry(bootTable)                                     //
          ),                                                                  //
      TelecommandHandler(UplinkProtocolDecoder, SupportedTelecommands.Get()), //
      CommDriver(fdir.ErrorCounting(), systemBus, TelecommandHandler)
{
}

void OBCCommunication::Initialize()
{
    this->CommDriver.Initialize();
}
