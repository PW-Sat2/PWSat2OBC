#include "adcs.hpp"
#include "base/reader.h"
#include "comm/ITransmitter.hpp"
#include "state/struct.h"
#include "telecommunication/downlink.h"

using telecommunication::downlink::DownlinkAPID;
using telecommunication::downlink::DownlinkFrame;

namespace obc
{
    namespace telecommands
    {
        SetBuiltinDetumblingBlockMaskTelecommand::SetBuiltinDetumblingBlockMaskTelecommand(IHasState<SystemState>& stateContainer_, //
            adcs::IAdcsCoordinator& adcsCoordinator_)
            : stateContainer(stateContainer_), //
              adcsCoordinator(adcsCoordinator_)
        {
        }

        void SetBuiltinDetumblingBlockMaskTelecommand::Handle(devices::comm::ITransmitter& transmitter, //
            gsl::span<const std::uint8_t> parameters)
        {
            DownlinkFrame response(DownlinkAPID::DisableBuiltinDetumbling, 0);
            auto& writer = response.PayloadWriter();
            auto& state = this->stateContainer.GetState().PersistentState;

            Reader r(parameters);
            auto newState = r.ReadByte() != 0;
            if (!r.Status())
            {
                writer.WriteByte(-1);
            }
            else if (!state.Set(state::AdcsState(newState)))
            {
                writer.WriteByte(-2);
            }
            else
            {
                this->adcsCoordinator.SetBlockMode(adcs::AdcsMode::BuiltinDetumbling, newState);
                writer.WriteByte(0);
            }

            transmitter.SendFrame(response.Frame());
        }

        SetAdcsModeTelecommand::SetAdcsModeTelecommand(adcs::IAdcsCoordinator& adcsCoordinator_) : adcsCoordinator(adcsCoordinator_)
        {
        }

        void SetAdcsModeTelecommand::Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters)
        {
            DownlinkFrame response(DownlinkAPID::SetAdcsMode, 0);
            auto& writer = response.PayloadWriter();

            Reader r(parameters);
            auto newState = static_cast<adcs::AdcsMode>(r.ReadByte());
            if (!r.Status())
            {
                Finish(OSResult::BufferNotAvailable, writer);
            }
            else
            {
                switch (newState)
                {
                    case adcs::AdcsMode::BuiltinDetumbling:
                        Finish(this->adcsCoordinator.EnableBuiltinDetumbling(), writer);
                        break;

                    case adcs::AdcsMode::ExperimentalDetumbling:
                        Finish(this->adcsCoordinator.EnableExperimentalDetumbling(), writer);
                        break;

                    case adcs::AdcsMode::ExperimentalSunpointing:
                        Finish(this->adcsCoordinator.EnableSunPointing(), writer);
                        break;

                    case adcs::AdcsMode::Disabled:
                        Finish(this->adcsCoordinator.Disable(), writer);
                        break;

                    default:
                        Finish(OSResult::InvalidArgument, writer);
                        break;
                }
            }

            transmitter.SendFrame(response.Frame());
        }

        void SetAdcsModeTelecommand::Finish(OSResult result, Writer& writer)
        {
            writer.WriteDoubleWordLE(num(result));
        }
    }
}
