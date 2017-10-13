#include "telemetry.hpp"
#include <algorithm>
#include "base/BitWriter.hpp"

using namespace std::chrono_literals;

namespace devices
{
    namespace antenna
    {
        ActivationCounts::ActivationCounts() : ActivationCounts(0, 0, 0, 0)
        {
        }

        ActivationCounts::ActivationCounts(std::uint8_t antenna1, std::uint8_t antenna2, std::uint8_t antenna3, std::uint8_t antenna4)
        {
            this->counts[0] = antenna1;
            this->counts[1] = antenna2;
            this->counts[2] = antenna3;
            this->counts[3] = antenna4;
        }

        void ActivationCounts::Write(BitWriter& writer) const
        {
            writer.WriteWord(std::min<std::uint8_t>(this->counts[0], 0b111), 3);
            writer.WriteWord(std::min<std::uint8_t>(this->counts[1], 0b111), 3);
            writer.WriteWord(std::min<std::uint8_t>(this->counts[2], 0b111), 3);
            writer.WriteWord(std::min<std::uint8_t>(this->counts[3], 0b111), 3);
        }

        ActivationTimes::ActivationTimes() : ActivationTimes(0s, 0s, 0s, 0s)
        {
        }

        ActivationTimes::ActivationTimes(std::chrono::seconds antenna1, //
            std::chrono::seconds antenna2,                              //
            std::chrono::seconds antenna3,                              //
            std::chrono::seconds antenna4                               //
            )
        {
            this->times[0] = antenna1;
            this->times[1] = antenna2;
            this->times[2] = antenna3;
            this->times[3] = antenna4;
        }

        void ActivationTimes::Write(BitWriter& writer) const
        {
            writer.WriteWord(std::min<std::uint16_t>(255, this->times[0].count() / 2), TimeLength);
            writer.WriteWord(std::min<std::uint16_t>(255, this->times[1].count() / 2), TimeLength);
            writer.WriteWord(std::min<std::uint16_t>(255, this->times[2].count() / 2), TimeLength);
            writer.WriteWord(std::min<std::uint16_t>(255, this->times[3].count() / 2), TimeLength);
        }

        AntennaTelemetry::AntennaTelemetry() : channelStatuses{ChannelStatus::None, ChannelStatus::None}
        {
        }

        static inline void SetAntennaStatus(
            std::bitset<8>& status, AntennaChannel channel, bool antenna1, bool antenna2, bool antenna3, bool antenna4)
        {
            auto idx = 4 * (channel - ANTENNA_FIRST_CHANNEL);

            if (idx > 4 || idx < 0)
            {
                return;
            }

            status[idx + 0] = antenna1;
            status[idx + 1] = antenna2;
            status[idx + 2] = antenna3;
            status[idx + 3] = antenna4;
        }

        void AntennaTelemetry::SetBurningStatus(AntennaChannel channel, bool antenna1, bool antenna2, bool antenna3, bool antenna4)
        {
            SetAntennaStatus(this->burnStatus, channel, antenna1, antenna2, antenna3, antenna4);
        }

        void AntennaTelemetry::SetDeployedStatus(AntennaChannel channel, bool antenna1, bool antenna2, bool antenna3, bool antenna4)
        {
            SetAntennaStatus(this->deployedStatus, channel, antenna1, antenna2, antenna3, antenna4);
        }

        void AntennaTelemetry::SetTimeReached(AntennaChannel channel, bool antenna1, bool antenna2, bool antenna3, bool antenna4)
        {
            SetAntennaStatus(this->timeReached, channel, antenna1, antenna2, antenna3, antenna4);
        }

        void AntennaTelemetry::Write(BitWriter& writer) const
        {
            writer.Write(this->deployedStatus);
            writer.Write(this->timeReached);
            writer.Write(this->burnStatus);

            writer.Write(has_flag(this->channelStatuses[0], ChannelStatus::IndependentBurn));
            writer.Write(has_flag(this->channelStatuses[1], ChannelStatus::IndependentBurn));
            writer.Write(has_flag(this->channelStatuses[0], ChannelStatus::IgnoringSwitches));
            writer.Write(has_flag(this->channelStatuses[1], ChannelStatus::IgnoringSwitches));
            writer.Write(has_flag(this->channelStatuses[0], ChannelStatus::Armed));
            writer.Write(has_flag(this->channelStatuses[1], ChannelStatus::Armed));

            this->activationCounts[0].Write(writer);
            this->activationCounts[1].Write(writer);
            this->activationTimes[0].Write(writer);
            this->activationTimes[1].Write(writer);
        }
    }
}
