#include "sail.hpp"
#include "base/reader.h"
#include "comm/ITransmitter.hpp"
#include "mission/sail.hpp"
#include "telecommunication/downlink.h"

using telecommunication::downlink::DownlinkAPID;
using telecommunication::downlink::CorrelatedDownlinkFrame;

namespace obc
{
    namespace telecommands
    {
        OpenSail::OpenSail(mission::IOpenSail& openSail) : _openSail(openSail)
        {
        }

        void OpenSail::Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters)
        {
            Reader r(parameters);
            auto correlationId = r.ReadByte();

            CorrelatedDownlinkFrame response(DownlinkAPID::Operation, 0, correlationId);

            if (!r.Status())
            {
                response.PayloadWriter().WriteByte(1);
                transmitter.SendFrame(response.Frame());
                return;
            }

            response.PayloadWriter().WriteByte(0);
            this->_openSail.OpenSail();

            transmitter.SendFrame(response.Frame());
        }
    }
}
