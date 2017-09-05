#include "comm.hpp"
#include <cmath>
#include "base/IHasState.hpp"
#include "base/reader.h"
#include "comm/ITransmitter.hpp"
#include "comm/comm.hpp"
#include "logger/logger.h"
#include "system.h"
#include "telecommunication/beacon.hpp"
#include "telecommunication/downlink.h"

using telecommunication::downlink::CorrelatedDownlinkFrame;
using telecommunication::downlink::DownlinkAPID;

namespace obc
{
    namespace telecommands
    {
        using namespace devices::comm;

        EnterIdleStateTelecommand::EnterIdleStateTelecommand(
            services::time::ICurrentTime& currentTime, mission::IIdleStateController& idleStateController)
            : _currentTime(currentTime), _idleStateController(idleStateController)
        {
        }

        void EnterIdleStateTelecommand::Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters)
        {
            Reader r(parameters);

            auto correlationId = r.ReadByte();
            auto duration = r.ReadByte();

            CorrelatedDownlinkFrame response(DownlinkAPID::Operation, 0, correlationId);

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

        SendBeaconTelecommand::SendBeaconTelecommand(IHasState<telemetry::TelemetryState>& provider) : telemetryState(provider)
        {
        }

        void SendBeaconTelecommand::Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> /*parameters*/)
        {
            telecommunication::downlink::RawFrame frame;
            auto& writer = frame.PayloadWriter();
            if (WriteBeaconPayload(this->telemetryState.GetState(), writer))
            {
                transmitter.SendFrame(frame.Frame());
            }
        }

        ResetTransmitterTelecommand::ResetTransmitterTelecommand()
        {
        }

        void ResetTransmitterTelecommand::Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> /*parameters*/)
        {
            const auto result = transmitter.ResetTransmitter();
            if (result)
            {
                LOG(LOG_LEVEL_INFO, "Resetting transmitter");
            }
            else
            {
                LOG(LOG_LEVEL_ERROR, "Unable to reset transmitter");
            }
        }
    }
}
