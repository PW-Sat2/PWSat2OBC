#include "power.hpp"
#include "base/reader.h"
#include "comm/ITransmitter.hpp"
#include "power/power.h"
#include "telecommunication/downlink.h"
#include "telecommunication/telecommand_handling.h"

using telecommunication::downlink::CorrelatedDownlinkFrame;
using telecommunication::downlink::DownlinkAPID;

namespace obc
{
    namespace telecommands
    {
        PowerCycle::PowerCycle(services::power::IPowerControl& powerControl) : _powerControl(powerControl)
        {
        }

        void PowerCycle::Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters)
        {
            Reader r(parameters);

            auto correlationId = r.ReadByte();

            if (!r.Status())
            {
                CorrelatedDownlinkFrame response(DownlinkAPID::Operation, 0, correlationId);
                response.PayloadWriter().WriteByte(0x1);

                transmitter.SendFrame(response.Frame());

                return;
            }

            {
                CorrelatedDownlinkFrame response(DownlinkAPID::Operation, 0, correlationId);
                response.PayloadWriter().WriteByte(0x0);

                transmitter.SendFrame(response.Frame());
            }

            this->_powerControl.PowerCycle();

            {
                CorrelatedDownlinkFrame response(DownlinkAPID::Operation, 0, correlationId);
                response.PayloadWriter().WriteByte(0x2);

                transmitter.SendFrame(response.Frame());
            }
        }
    }
}
