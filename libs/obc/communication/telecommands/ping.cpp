#include "ping.hpp"

#include "telecommunication/downlink.h"

#include "comm/ITransmitter.hpp"

using std::uint8_t;
using gsl::span;
using devices::comm::ITransmitter;
using telecommunication::downlink::DownlinkFrame;
using telecommunication::downlink::DownlinkAPID;

namespace obc
{
    namespace telecommands
    {
        void PingTelecommand::Handle(ITransmitter& transmitter, span<const uint8_t> parameters)
        {
            UNREFERENCED_PARAMETER(parameters);

            const char* response = "PONG";

            DownlinkFrame frame(DownlinkAPID::Pong, 0);

            frame.PayloadWriter().WriteArray(gsl::make_span(reinterpret_cast<const uint8_t*>(response), 4));

            transmitter.SendFrame(frame.Frame());
        }
    }
}
