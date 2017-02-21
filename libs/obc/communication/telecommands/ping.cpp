#include "ping.hpp"

#include "telecommunication/downlink.h"

#include "comm/ITransmitFrame.hpp"

using std::uint8_t;
using gsl::span;
using devices::comm::ITransmitFrame;
using telecommunication::downlink::DownlinkFrame;
using telecommunication::downlink::DownlinkAPID;

namespace obc
{
    namespace telecommands
    {
        uint8_t PingTelecommand::CommandCode() const
        {
            return static_cast<uint8_t>('P');
        }

        void PingTelecommand::Handle(ITransmitFrame& transmitter, span<const uint8_t> parameters)
        {
            UNREFERENCED_PARAMETER(parameters);

            const char* response = "PONG";

            DownlinkFrame frame(DownlinkAPID::Pong, 0);

            frame.PayloadWriter().WriteArray(gsl::make_span(reinterpret_cast<const uint8_t*>(response), 4));

            transmitter.SendFrame(frame.Frame());
        }
    }
}
