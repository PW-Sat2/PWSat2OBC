#include "communication.h"
#include "gsl/span"
#include "settings.h"
#include "telecommunication/downlink.h"

using std::uint8_t;
using gsl::span;
using drivers::i2c::II2CBus;
using devices::comm::ITransmitFrame;
using obc::PingTelecommand;
using telecommands::handling::IHandleTeleCommand;
using telecommunication::DownlinkFrame;
using telecommunication::APID;

using namespace obc;

void PingTelecommand::Handle(ITransmitFrame& transmitter, span<const uint8_t> parameters)
{
    UNREFERENCED_PARAMETER(parameters);

    const char* response = "PONG";

    DownlinkFrame frame(APID::Pong, 0);

    frame.PayloadWriter().WriteArray(gsl::make_span(reinterpret_cast<const uint8_t*>(response), 4));

    transmitter.SendFrame(frame.Frame());
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
