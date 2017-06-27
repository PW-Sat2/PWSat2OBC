#include "antenna.hpp"
#include "base/reader.h"
#include "comm/ITransmitter.hpp"
#include "telecommunication/downlink.h"

using telecommunication::downlink::CorrelatedDownlinkFrame;
using telecommunication::downlink::DownlinkAPID;

namespace obc
{
    namespace telecommands
    {
        StopAntennaDeployment::StopAntennaDeployment(mission::antenna::IDisableAntennaDeployment& disableAntennaDeployment)
            : _disableAntennaDeployment(disableAntennaDeployment)
        {
        }

        void StopAntennaDeployment::Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters)
        {
            Reader r(parameters);

            auto correlationId = r.ReadByte();

            CorrelatedDownlinkFrame response(DownlinkAPID::Operation, 0, correlationId);

            if (!r.Status())
            {
                response.PayloadWriter().WriteByte(-1);

                transmitter.SendFrame(response.Frame());
                return;
            }

            this->_disableAntennaDeployment.DisableDeployment();

            response.PayloadWriter().WriteByte(0);

            transmitter.SendFrame(response.Frame());
        }
    }
}
