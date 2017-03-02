#include "communication.h"
#include "gsl/span"
#include "settings.h"
#include "telecommunication/downlink.h"

using std::uint8_t;
using gsl::span;
using drivers::i2c::II2CBus;
using telecommunication::uplink::IHandleTeleCommand;

using namespace obc;

Telecommands::Telecommands(services::fs::IFileSystem& fs)
    : _ping(),                      //
      _downloadFileTelecommand(fs), //
      _telecommands{&_ping, &_downloadFileTelecommand}
{
}

gsl::span<IHandleTeleCommand*> Telecommands::AllTelecommands()
{
    return gsl::span<IHandleTeleCommand*>(this->_telecommands);
}

OBCCommunication::OBCCommunication(obc::FDIR& fdir, II2CBus& systemBus, services::fs::IFileSystem& fs)
    : UplinkProtocolDecoder(settings::CommSecurityCode),                                  //
      SupportedTelecommands(fs),                                                          //
      TelecommandHandler(UplinkProtocolDecoder, SupportedTelecommands.AllTelecommands()), //
      CommDriver(fdir.ErrorCounting(), systemBus, TelecommandHandler)
{
}

void OBCCommunication::Initialize()
{
    this->CommDriver.Initialize();
}
