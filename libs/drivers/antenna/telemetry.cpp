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
            writer.WriteWord(this->times[0].count(), TimeLength);
            writer.WriteWord(this->times[1].count(), TimeLength);
            writer.WriteWord(this->times[2].count(), TimeLength);
            writer.WriteWord(this->times[3].count(), TimeLength);
        }

        AntennaTelemetry::AntennaTelemetry() : deploymentStatus(0)
        {
        }

        void AntennaTelemetry::Write(BitWriter& writer) const
        {
            writer.Write(this->deploymentStatus);
            this->activationCounts[0].Write(writer);
            this->activationCounts[1].Write(writer);
            this->activationTimes[0].Write(writer);
            this->activationTimes[1].Write(writer);
        }

        void AntennaTelemetry::SetDeploymentStatus(AntennaChannel channel, AntennaId antenna, bool status)
        {
            const std::uint8_t shift = (channel - ANTENNA_FIRST_CHANNEL) * 4;
            const std::uint8_t value = (1 << (antenna - ANTENNA1_ID)) << shift;
            if (status)
            {
                this->deploymentStatus |= value;
            }
            else
            {
                this->deploymentStatus &= ~value;
            }
        }

        bool AntennaTelemetry::GetDeploymentStatus(AntennaChannel channel, AntennaId antenna) const
        {
            const std::uint8_t shift = (channel - ANTENNA_FIRST_CHANNEL) * 4;
            const std::uint8_t value = (1 << (antenna - ANTENNA1_ID)) << shift;
            return (this->deploymentStatus & value) != 0;
        }
    }
}
