#include "communication.h"

using std::uint8_t;
using gsl::span;
using drivers::i2c::II2CBus;
using devices::comm::ITransmitFrame;
using obc::PingTelecommand;
using telecommands::handling::IHandleTeleCommand;

using namespace obc;

void PingTelecommand::Handle(ITransmitFrame& transmitter, span<const uint8_t> parameters)
{
    UNREFERENCED_PARAMETER(parameters);

    const char* response = "PONG";

    transmitter.SendFrame(span<const uint8_t>(reinterpret_cast<const uint8_t*>(response), 4));
}

uint8_t PingTelecommand::CommandCode() const
{
    return static_cast<uint8_t>('P');
}

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
    : UplinkProtocolDecoder(0xDDCCBBAA),                                                  //
      SupportedTelecommands(),                                                            //
      TelecommandHandler(UplinkProtocolDecoder, SupportedTelecommands.AllTelecommands()), //
      CommDriver(systemBus, TelecommandHandler)
{
}

void OBCCommunication::Initialize()
{
    this->CommDriver.Initialize();
}
