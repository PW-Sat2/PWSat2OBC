#include "communication.h"

using namespace communication;
using namespace std;
using telecommands::handling::IHandleTeleCommand;

PingTelecommand::PingTelecommand(devices::comm::CommObject& comm) : _comm(comm)
{
}

void PingTelecommand::Handle(gsl::span<const uint8_t> parameters)
{
    UNREFERENCED_PARAMETER(parameters);

    const char* response = "PONG";

    this->_comm.SendFrame(gsl::span<const uint8_t>(reinterpret_cast<const uint8_t*>(response), 4));
}

std::uint8_t PingTelecommand::CommandCode() const
{
    return static_cast<uint8_t>('P');
}

Telecommands::Telecommands(devices::comm::CommObject& comm)
    : _ping(comm), //
      _telecommands{&_ping}
{
}

gsl::span<IHandleTeleCommand*> Telecommands::AllTelecommands()
{
    return gsl::span<IHandleTeleCommand*>(this->_telecommands);
}

OBCCommunication::OBCCommunication(I2CBus& systemBus)
    : UplinkProtocolDecoder(),                                                                                   //
      SupportedTelecommands(CommDriver),                                                                         //
      TelecommandHandler(UplinkProtocolDecoder, UplinkProtocolDecoder, SupportedTelecommands.AllTelecommands()), //
      CommDriver(systemBus, TelecommandHandler)
{
}

void OBCCommunication::Initialize()
{
    this->CommDriver.Initialize();
}
