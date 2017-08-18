#include "state.hpp"
#include "comm/ITransmitter.hpp"
#include "telecommunication/downlink.h"

namespace obc
{
    namespace telecommands
    {
        using telecommunication::downlink::DownlinkFrame;
        using telecommunication::downlink::DownlinkAPID;

        static bool WriteState(gsl::span<std::uint8_t> target, const state::SystemPersistentState& state)
        {
            Writer writer(target);
            if (!state.Write(writer))
            {
                return false;
            }

            return writer.Status();
        }

        GetPersistentStateTelecommand::GetPersistentStateTelecommand(IHasState<SystemState>& stateContainer_)
            : stateContainer(stateContainer_)
        {
        }

        void GetPersistentStateTelecommand::Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> /*parameters*/)
        {
            std::uint32_t seq = 0;
            std::array<std::uint8_t, state::SystemPersistentState::Size()> buffer;
            if (!WriteState(buffer, this->stateContainer.GetState().PersistentState))
            {
                return;
            }

            Reader reader(buffer);
            while (reader.RemainingSize() > 0)
            {
                DownlinkFrame frame{DownlinkAPID::PersistentState, seq};
                auto& writer = frame.PayloadWriter();
                writer.WriteArray(reader.ReadArray(gsl::narrow_cast<std::uint16_t>(writer.RemainingSize())));
                transmitter.SendFrame(frame.Frame());
                ++seq;
            }
        }
    }
}
