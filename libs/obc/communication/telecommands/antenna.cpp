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
        SetAntennaDeploymentMaskTelecommand::SetAntennaDeploymentMaskTelecommand(
            mission::antenna::IDisableAntennaDeployment& disableAntennaDeployment)
            : _disableAntennaDeployment(disableAntennaDeployment)
        {
        }

        void SetAntennaDeploymentMaskTelecommand::Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters)
        {
            Reader r(parameters);

            auto correlationId = r.ReadByte();
            auto disabled = r.ReadByte() == 0;
            CorrelatedDownlinkFrame response(DownlinkAPID::Operation, 0, correlationId);
            if (!r.Status())
            {
                response.PayloadWriter().WriteByte(-1);
            }
            else
            {
                this->_disableAntennaDeployment.SetDeploymentState(disabled);
                response.PayloadWriter().WriteByte(0);
            }

            transmitter.SendFrame(response.Frame());
        }
    }
}
