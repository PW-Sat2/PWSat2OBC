#ifndef LIBS_TELEMETRY_INCLUDE_TELEMETRY_GPIO_HPP_
#define LIBS_TELEMETRY_INCLUDE_TELEMETRY_GPIO_HPP_

#pragma once

#include "base/fwd.hpp"

namespace telemetry
{
    /**
     * @brief This class represents the state that is observed by the mcu via its gpios.
     * @telemetry_element
     */
    class GpioState
    {
      public:
        /**
         * @brief ctor.
         * @param sailState Sail deployment state.
         */
        GpioState(bool sailState = false);

        /**
         * @brief Write the system error counting element to passed buffer writer object.
         * @param[in] writer Buffer writer object that should be used to write the serialized state.
         */
        void Write(BitWriter& writer) const;

        /**
         * @brief Returns size of the serialized state in bits.
         * @return Size of the serialized state in bits.
         */
        static constexpr std::uint32_t BitSize();

        /**
         * @brief Returns information regarding current sail state.
         * @return Sail deployment state.
         */
        bool IsSailDeployed() const;

        /**
         * @brief Updates sail deployment state.
         * @param newState New sail deployment state.
         */
        void SetSailDeploymentState(bool newState);

      private:
        bool isDeployed;
    };

    constexpr std::uint32_t GpioState::BitSize()
    {
        return 1;
    }

    inline bool GpioState::IsSailDeployed() const
    {
        return this->isDeployed;
    }

    inline void GpioState::SetSailDeploymentState(bool newState)
    {
        this->isDeployed = newState;
    }

    static_assert(GpioState::BitSize() == 1, "Invalid serialized size");
}

#endif /* LIBS_TELEMETRY_INCLUDE_TELEMETRY_GPIO_HPP_ */
