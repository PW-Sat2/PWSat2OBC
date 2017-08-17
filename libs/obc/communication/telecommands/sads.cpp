#include "sads.hpp"
#include "base/reader.h"
#include "comm/ITransmitter.hpp"
#include "mission/sads.hpp"
#include "telecommunication/downlink.h"

using telecommunication::downlink::DownlinkAPID;
using telecommunication::downlink::CorrelatedDownlinkFrame;

namespace obc
{
    namespace telecommands
    {
        DeploySolarArray::DeploySolarArray(mission::IDeploySolarArray& deploySolarArray) : _deploySolarArray(deploySolarArray)
        {
        }

        void DeploySolarArray::Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters)
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
            this->_deploySolarArray.DeploySolarArray();

            transmitter.SendFrame(response.Frame());
        }
    }
}
