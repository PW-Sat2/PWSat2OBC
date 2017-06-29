#include "antenna/AntennaConfiguration.hpp"
#include "antenna/AntennaState.hpp"
#include "base/reader.h"
#include "base/writer.h"
#include "logger/logger.h"

namespace state
{
    AntennaState::AntennaState() : deployed(false)
    {
    }

    AntennaConfiguration::AntennaConfiguration() //
        : isDeploymentDisabled(false)
    {
    }

    AntennaConfiguration::AntennaConfiguration(bool deploymentDisabled) //
        : isDeploymentDisabled(deploymentDisabled)
    {
        if (!isDeploymentDisabled)
        {
            LOG(LOG_LEVEL_INFO, "[TUTUAJ]");
        }
    }

    void AntennaConfiguration::Read(Reader& reader)
    {
        const bool value = reader.ReadByte() != 0;
        if (reader.Status())
        {
            this->isDeploymentDisabled = value;
        }
    }

    void AntennaConfiguration::Write(Writer& writer) const
    {
        writer.WriteByte(static_cast<std::uint8_t>(this->isDeploymentDisabled));
    }
}
