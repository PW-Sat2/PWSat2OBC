#include "communication.h"
#include "gsl/span"
#include "settings.h"
#include "telecommunication/downlink.h"

using std::uint8_t;
using gsl::span;
using drivers::i2c::II2CBus;
using telecommunication::uplink::IHandleTeleCommand;

using namespace obc;

Telecommands::Telecommands()
    : _ping(), //
      _telecommands{&_ping}
{
}

gsl::span<IHandleTeleCommand*> Telecommands::AllTelecommands()
{
    return gsl::span<IHandleTeleCommand*>(this->_telecommands);
}

OBCCommunication::OBCCommunication(II2CBus& systemBus)
    : UplinkProtocolDecoder(settings::CommSecurityCode),                                  //
      SupportedTelecommands(),                                                            //
      TelecommandHandler(UplinkProtocolDecoder, SupportedTelecommands.AllTelecommands()), //
      CommDriver(systemBus, TelecommandHandler)
{
}

void OBCCommunication::Initialize()
{
    this->CommDriver.Initialize();
}
