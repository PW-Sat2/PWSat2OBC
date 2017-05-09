#include "comm.hpp"
#include <cmath>
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
        EnterIdleStateTelecommand::EnterIdleStateTelecommand(
            services::time::ICurrentTime& currentTime, mission::IIdleStateController& idleStateController)
            : _currentTime(currentTime), _idleStateController(idleStateController)
        {
        }

        void EnterIdleStateTelecommand::Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters)
        {
            using namespace devices::comm;

            Reader r(parameters);

            auto correlationId = r.ReadByte();
            auto duration = r.ReadByte();

            DownlinkFrame response(DownlinkAPID::Operation, 0);
            response.PayloadWriter().WriteByte(correlationId);

            if (!r.Status())
            {
                LOG(LOG_LEVEL_ERROR, "Malformed request");
                response.PayloadWriter().WriteByte(-1);
                transmitter.SendFrame(response.Frame());
                return;
            }

            LOGF(LOG_LEVEL_INFO, "Entering idle state for %d seconds", duration);

            auto currentTime = _currentTime.GetCurrentTime();
            if (!currentTime.HasValue)
            {
                LOG(LOG_LEVEL_ERROR, "Unable to enter idle state - current time is unavailable");
                response.PayloadWriter().WriteByte(-1);
                transmitter.SendFrame(response.Frame());
                return;
            }

            auto disableTime = currentTime.Value + std::chrono::seconds(duration);

            const auto status = _idleStateController.EnterTransmitterStateWhenIdle(disableTime);
            if (status)
            {
                LOG(LOG_LEVEL_DEBUG, "Entered idle state");
                response.PayloadWriter().WriteByte(0);
            }
            else
            {
                LOG(LOG_LEVEL_ERROR, "Unable to enter idle state");
                response.PayloadWriter().WriteByte(-1);
            }

            transmitter.SendFrame(response.Frame());
        }
    }
}
