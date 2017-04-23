#include "communication.h"
#include "gsl/span"
#include "settings.h"
#include "telecommunication/downlink.h"

using std::uint8_t;
using gsl::span;
using drivers::i2c::II2CBus;
using telecommunication::uplink::IHandleTeleCommand;

using namespace obc;

Telecommands::Telecommands(services::fs::IFileSystem& fs, obc::OBCExperiments& experiments)
    : _ping(),                                   //
      _downloadFileTelecommand(fs),              //
      _performDetumblingExperiment(experiments), //
      _abortExperiment(experiments),             //
      _telecommands{
          &_ping,                        //
          &_downloadFileTelecommand,     //
          &_performDetumblingExperiment, //
          &_abortExperiment,             //
      }
{
}

gsl::span<IHandleTeleCommand*> Telecommands::AllTelecommands()
{
    return gsl::span<IHandleTeleCommand*>(this->_telecommands);
}

OBCCommunication::OBCCommunication(obc::FDIR& fdir, II2CBus& systemBus, services::fs::IFileSystem& fs, obc::OBCExperiments& experiments)
    : UplinkProtocolDecoder(settings::CommSecurityCode),                                  //
      SupportedTelecommands(fs, experiments),                                             //
      TelecommandHandler(UplinkProtocolDecoder, SupportedTelecommands.AllTelecommands()), //
      CommDriver(fdir.ErrorCounting(), systemBus, TelecommandHandler)
{
}

void OBCCommunication::Initialize()
{
    this->CommDriver.Initialize();
}
