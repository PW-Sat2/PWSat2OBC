#include "time.hpp"
#include "base/reader.h"
#include "comm/ITransmitter.hpp"
#include "comm/comm.hpp"
#include "logger/logger.h"
#include "system.h"
#include "telecommunication/downlink.h"

using telecommunication::downlink::DownlinkFrame;
using telecommunication::downlink::DownlinkAPID;

namespace obc
{
    namespace telecommands
    {
        SetTimeCorrectionConfigTelecommand::SetTimeCorrectionConfigTelecommand(IHasState<SystemState>& stateContainer_)
            : stateContainer(stateContainer_)
        {
        }

        void SetTimeCorrectionConfigTelecommand::Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters)
        {
            using namespace devices::comm;

            Reader r(parameters);

            auto correlationId = r.ReadByte();
            auto missionTimeWeight = r.ReadWordLE();
            auto externalTimeWeight = r.ReadWordLE();

            DownlinkFrame response(DownlinkAPID::Operation, 0);
            response.PayloadWriter().WriteByte(correlationId);

            if (missionTimeWeight == 0 && externalTimeWeight == 0)
            {
                LOG(LOG_LEVEL_WARNING, "Cannot set time correction configuration with both weights equal to 0");
                response.PayloadWriter().WriteByte(-1);
                transmitter.SendFrame(response.Frame());
                return;
            }

            LOGF(LOG_LEVEL_INFO,
                "Setting time correction configuration. Mission time weight %d, External time weight %d",
                missionTimeWeight,
                externalTimeWeight);

            auto& persistentState = stateContainer.GetState().PersistentState;
            persistentState.Set(state::TimeCorrectionConfiguration(missionTimeWeight, externalTimeWeight));

            response.PayloadWriter().WriteByte(0);
            transmitter.SendFrame(response.Frame());
        }
    }
}
