#ifndef LIBS_STATE_ANTENNA_ANTENNA_CONFIGURATION_HPP
#define LIBS_STATE_ANTENNA_ANTENNA_CONFIGURATION_HPP

#pragma once

#include <cstdint>
#include "base/fwd.hpp"

/**
 * @addtogroup StateDef
 * @{
 */
namespace state
{
    /**
     * @brief This type contains antenna deployment process configuration.
     */
    class AntennaConfiguration
    {
      public:
        /** @brief ctor. */
        AntennaConfiguration();

        /**
         * @brief ctor.
         * @param[in] deploymentDisabled Flag indicating whether the deployment process is disabled.
         */
        explicit AntennaConfiguration(bool deploymentDisabled);

        /**
         * @brief Returns Information whether the antenna deployment process is disabled.
         * @return True if antenna deployment process is disabled, false otherwise.
         */
        bool IsDeploymentDisabled() const;

        /**
         * @brief Read antenna deployment configuration from passed reader.
         * @param[in] reader Buffer reader that should be used to read the antenna configuration serialized state.
         */
        void Read(Reader& reader);

        /**
         * @brief Write antenna configuration to passed buffer writer object.
         * @param[in] writer Buffer writer object that should be used to write serialized state
         * of the antenna deployment configuration process.
         */
        void Write(Writer& writer) const;

        /**
         * @brief Returns size of the serialized state in bytes.
         * @return Size of the serialized state in bytes.
         */
        static constexpr std::uint32_t Size();

        /**
         * @brief Equality operator.
         * @param[in] arg Object to compare to.
         * @return Operation status.
         */
        bool operator==(const AntennaConfiguration& arg) const;

        /**
         * @brief Inequality operator.
         * @param[in] arg Object to compare to.
         * @return Operation status.
         */
        bool operator!=(const AntennaConfiguration& arg) const;

      private:
        /**
         * Flag indicating whether the deployment process is disabled.
         *
         * True if antenna deployment process is disabled, false otherwise.
         */
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

    inline bool AntennaConfiguration::operator==(const AntennaConfiguration& arg) const
    {
        return this->isDeploymentDisabled == arg.isDeploymentDisabled;
    }

    inline bool AntennaConfiguration::operator!=(const AntennaConfiguration& arg) const
    {
        return !(*this == arg);
    }
}

/** @} */
#endif
