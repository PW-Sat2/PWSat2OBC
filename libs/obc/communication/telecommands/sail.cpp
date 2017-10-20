#include "sail.hpp"
#include "base/reader.h"
#include "comm/ITransmitter.hpp"
#include "mission/sail.hpp"
#include "telecommunication/downlink.h"

using telecommunication::downlink::CorrelatedDownlinkFrame;
using telecommunication::downlink::DownlinkAPID;

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
            auto ignoreOverheat = r.ReadByte() != 0;

            CorrelatedDownlinkFrame response(DownlinkAPID::Sail, 0, correlationId);

            if (!r.Status())
            {
                response.PayloadWriter().WriteByte(1);
            }
            else
            {
                response.PayloadWriter().WriteByte(0);
                this->_openSail.OpenSail(ignoreOverheat);
            }

            transmitter.SendFrame(response.Frame());
        }

        StopSailDeployment::StopSailDeployment(IHasState<SystemState>& stateContainer) : stateContainer(stateContainer)
        {
        }

        void StopSailDeployment::Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters)
        {
            Reader r(parameters);

            auto correlationId = r.ReadByte();
            auto& persistentState = stateContainer.GetState().PersistentState;
            CorrelatedDownlinkFrame response(DownlinkAPID::DisableSailDeployment, 0, correlationId);

            if (!r.Status())
            {
                response.PayloadWriter().WriteByte(-1);
            }
            else if (!persistentState.Set(state::SailState(state::SailOpeningState::OpeningStopped)))
            {
                LOG(LOG_LEVEL_ERROR, "[sail] Can't set sail state");
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
