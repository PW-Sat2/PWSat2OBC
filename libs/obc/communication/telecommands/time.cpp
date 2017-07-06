#include "time.hpp"
#include "base/reader.h"
#include "comm/ITransmitter.hpp"
#include "comm/comm.hpp"
#include "logger/logger.h"
#include "system.h"
#include "telecommunication/downlink.h"

using telecommunication::downlink::DownlinkFrame;
using telecommunication::downlink::DownlinkAPID;
using services::time::ICurrentTime;
using devices::rtc::IRTC;
using devices::rtc::RTCTime;

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
            if (!persistentState.Set(state::TimeCorrectionConfiguration(missionTimeWeight, externalTimeWeight)))
            {
                LOG(LOG_LEVEL_ERROR, "Cannot set time correction configuration state");
                response.PayloadWriter().WriteByte(-2);
                transmitter.SendFrame(response.Frame());
                return;
            }

            response.PayloadWriter().WriteByte(0);
            transmitter.SendFrame(response.Frame());
        }

        SetTimeTelecommand::SetTimeTelecommand(IHasState<SystemState>& stateContainer_,
            ICurrentTime& timeProvider_,
            IRTC& rtc_,
            mission::ITimeSynchronization& timeSynchronization_)
            : stateContainer(stateContainer_), timeProvider(timeProvider_), rtc(rtc_), timeSynchronization(timeSynchronization_)
        {
        }

        void SetTimeTelecommand::Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters)
        {
            using namespace devices::comm;

            Reader r(parameters);

            auto correlationId = r.ReadByte();
            auto targetTime = std::chrono::seconds(r.ReadDoubleWordLE());

            DownlinkFrame response(DownlinkAPID::Operation, 0);
            response.PayloadWriter().WriteByte(correlationId);

            if (!this->timeSynchronization.AcquireTimeSynchronizationLock())
            {
                LOG(LOG_LEVEL_ERROR, "Unable to acquire time synchronization lock");
                response.PayloadWriter().WriteByte(-4);
                transmitter.SendFrame(response.Frame());
                return;
            }

            devices::rtc::RTCTime rtcTime;
            if (OS_RESULT_FAILED(rtc.ReadTime(rtcTime)))
            {
                LOG(LOG_LEVEL_ERROR, "Unable to retrieve time from external RTC");
                response.PayloadWriter().WriteByte(-1);
                transmitter.SendFrame(response.Frame());
                return;
            }

            const auto externalTime = std::chrono::duration_cast<std::chrono::milliseconds>(rtcTime.ToDuration());

            LOGF(LOG_LEVEL_INFO,
                "Jumping to time 0x%x%xms 0x%x%xms\n",
                static_cast<unsigned int>(targetTime.count() >> 32),
                static_cast<unsigned int>(targetTime.count()),
                static_cast<unsigned int>(externalTime.count() >> 32),
                static_cast<unsigned int>(externalTime.count()));

            if (!timeProvider.SetCurrentTime(targetTime))
            {
                LOG(LOG_LEVEL_ERROR, "Unable to set time");
                response.PayloadWriter().WriteByte(-2);
                transmitter.SendFrame(response.Frame());
                return;
            }

            auto& persistentState = stateContainer.GetState().PersistentState;
            if (!persistentState.Set(state::TimeState(targetTime, externalTime)))
            {
                LOG(LOG_LEVEL_ERROR, "Cannot set time state");
                response.PayloadWriter().WriteByte(-3);
                transmitter.SendFrame(response.Frame());
                return;
            }

            if (!this->timeSynchronization.ReleaseTimeSynchronizationLock())
            {
                LOG(LOG_LEVEL_ERROR, "Unable to release time synchronization lock");
                response.PayloadWriter().WriteByte(-5);
                transmitter.SendFrame(response.Frame());
                return;
            }

            response.PayloadWriter().WriteByte(0);
            transmitter.SendFrame(response.Frame());
        }
    }
}
