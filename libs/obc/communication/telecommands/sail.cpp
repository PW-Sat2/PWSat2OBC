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

            CorrelatedDownlinkFrame response(DownlinkAPID::Sail, 0, correlationId);

            if (!r.Status())
            {
                response.PayloadWriter().WriteByte(1);
                transmitter.SendFrame(response.Frame());
                return;
            }

            response.PayloadWriter().WriteByte(0);
            this->_openSail.OpenSail(true);

            transmitter.SendFrame(response.Frame());
        }

        StopSailDeployment::StopSailDeployment(IHasState<SystemState>& stateContainer) : stateContainer(stateContainer)
        {
        }

        void StopSailDeployment::Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters)
        {
            Reader r(parameters);

            auto correlationId = r.ReadByte();

            CorrelatedDownlinkFrame response(DownlinkAPID::DisableSailDeployment, 0, correlationId);

            if (!r.Status())
            {
                LOG(LOG_LEVEL_ERROR, "Can't get sail state");
                response.PayloadWriter().WriteByte(-1);

                transmitter.SendFrame(response.Frame());
                return;
            }

            auto& state = stateContainer.GetState();
            auto& persistentState = state.PersistentState;

            state::SailState sailState;
            if (!persistentState.Get(sailState))
            {
                LOG(LOG_LEVEL_ERROR, "Can't get sail state");
                response.PayloadWriter().WriteByte(-2);

                transmitter.SendFrame(response.Frame());
                return;
            }

            auto currentState = sailState.CurrentState();
            if (currentState == state::SailOpeningState::Opening || state.SailOpened)
            {
                LOG(LOG_LEVEL_ERROR, "Can't get sail state");
                response.PayloadWriter().WriteByte(-3);

                transmitter.SendFrame(response.Frame());
                return;
            }

            if (!persistentState.Set(state::SailState(state::SailOpeningState::OpeningStopped)))
            {
                LOG(LOG_LEVEL_ERROR, "[sail] Can't set sail state");
                response.PayloadWriter().WriteByte(-4);

                transmitter.SendFrame(response.Frame());
                return;
            }

            response.PayloadWriter().WriteByte(0);

            transmitter.SendFrame(response.Frame());
        }
    }
}
