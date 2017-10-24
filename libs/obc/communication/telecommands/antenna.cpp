#include "antenna.hpp"
#include "base/IHasState.hpp"
#include "base/reader.h"
#include "comm/ITransmitter.hpp"
#include "telecommunication/downlink.h"

using telecommunication::downlink::CorrelatedDownlinkFrame;
using telecommunication::downlink::DownlinkAPID;

namespace obc
{
    namespace telecommands
    {
        SetAntennaDeploymentMaskTelecommand::SetAntennaDeploymentMaskTelecommand(IHasState<SystemState>& stateContainer)
            : stateContainer(stateContainer)
        {
        }

        void SetAntennaDeploymentMaskTelecommand::Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters)
        {
            Reader r(parameters);

            auto& persistentState = stateContainer.GetState().PersistentState;
            auto correlationId = r.ReadByte();
            auto disabled = r.ReadByte() != 0;
            CorrelatedDownlinkFrame response(DownlinkAPID::DisableAntennaDeployment, 0, correlationId);
            if (!r.Status())
            {
                response.PayloadWriter().WriteByte(-1);
            }
            else if (!persistentState.Set(state::AntennaConfiguration(disabled)))
            {
                LOG(LOG_LEVEL_ERROR, "[antenna][tc] Unable to disable antenna deployment");
                response.PayloadWriter().WriteByte(-2);
            }
            else
            {
                response.PayloadWriter().WriteByte(0);
            }

            transmitter.SendFrame(response.Frame());
        }
    }
}
