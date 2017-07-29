#include "periodic_message.hpp"
#include "comm/ITransmitter.hpp"
#include "state/struct.h"

using telecommunication::downlink::DownlinkAPID;
using telecommunication::downlink::CorrelatedDownlinkFrame;

namespace obc
{
    namespace telecommands
    {
        SetPeriodicMessageTelecommand::SetPeriodicMessageTelecommand(IHasState<SystemState>& stateContainer)
            : _stateContainer(stateContainer)
        {
        }

        void SetPeriodicMessageTelecommand::Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters)
        {
            Reader r(parameters);

            auto correlationId = r.ReadByte();

            CorrelatedDownlinkFrame response(DownlinkAPID::Operation, 0, correlationId);

            auto interval = std::chrono::minutes(r.ReadByte());
            auto repeatCount = r.ReadByte();

            auto msg = r.ReadToEnd();

            if (!r.Status() || msg.size() == 0)
            {
                response.PayloadWriter().WriteByte(1);
                transmitter.SendFrame(response.Frame());
                return;
            }

            state::MessageState newSettings(interval, repeatCount, msg);

            auto& persistentState = this->_stateContainer.GetState().PersistentState;
            persistentState.Set(newSettings);

            response.PayloadWriter().WriteByte(0);

            transmitter.SendFrame(response.Frame());
        }
    }
}
