#ifndef LIBS_STATE_ANTENNA_ANTENNA_CONFIGURATION_HPP
#define LIBS_STATE_ANTENNA_ANTENNA_CONFIGURATION_HPP

#pragma once

#include <cstdint>
#include "base/fwd.hpp"

namespace state
{
    class AntennaConfiguration
    {
      public:
        AntennaConfiguration();

        explicit AntennaConfiguration(bool deploymentDisabled);

        bool IsDeploymentDisabled() const;

        void Read(Reader& reader);

        void Write(Writer& writer) const;

        static constexpr std::uint32_t Size();

      private:
        bool isDeploymentDisabled;
    };

    inline bool AntennaConfiguration::IsDeploymentDisabled() const
    {
        return isDeploymentDisabled;
    }

    inline constexpr std::uint32_t AntennaConfiguration::Size()
    {
        return 1;
    }
}

#endif
