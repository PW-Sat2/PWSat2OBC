#ifndef LIBS_TELEMETRY_SYSTEM_STARTUP_HPP
#define LIBS_TELEMETRY_SYSTEM_STARTUP_HPP

#pragma once

#include <cstdint>
#include "base/fwd.hpp"

namespace telemetry
{
    /**
     * @brief This type represents telemetry element related to the system startup event.
     * @telemetry_element
     * @ingroup telemetry
     *
     * This type records some useful information regarding system startup.
     */
    class SystemStartup
    {
      public:
        /**
         * @brief SystemStartup telemetry unique identifier.
         */
        static constexpr int Id = 1;

        /**
         * @brief ctor.
         */
        SystemStartup();

        /**
         * @brief ctor.
         * @param[in] counter Current boot counter value.
         * @param[in] index Currently used boot index.
         * @param[in] reason Reason of last mcu reset.
         */
        SystemStartup(std::uint32_t counter, std::uint8_t index, std::uint32_t reason);

        /**
         * @brief Returns current boot counter.
         * @return Current boot counter.
         */
        std::uint32_t BootCounter() const noexcept;

        /**
         * @brief Returns current boot index.
         * @return Current boot index.
         */
        std::uint8_t BootIndex() const noexcept;

        /**
         * @brief Returns reason of the last mcu reset.
         * @return reason of the last mcu reset.
         */
        std::uint32_t BootReason() const noexcept;

        /**
         * @brief Write the system startup telemetry element to passed buffer writer object.
         * @param[in] writer Buffer writer object that should be used to write the serialized state.
         */
        void Write(BitWriter& writer) const;

        /**
         * @brief Returns size of the serialized state in bytes.
         * @return Size of the serialized state in bytes.
         */
        static constexpr std::uint32_t BitSize();

      private:
        /**
         * @brief Current boot counter value.
         */
        std::uint32_t bootCounter;

        /**
         * @brief Reason of the last mcu reset
         */
        std::uint32_t bootReason;

        /**
         * @brief Current boot index.
         */
        std::uint8_t bootIndex;
    };

    inline std::uint32_t SystemStartup::BootCounter() const noexcept
    {
        return this->bootCounter;
    }

    inline std::uint8_t SystemStartup::BootIndex() const noexcept
    {
        return this->bootIndex;
    }

    inline std::uint32_t SystemStartup::BootReason() const noexcept
    {
        return this->bootReason;
    }

    constexpr std::uint32_t SystemStartup::BitSize()
    {
        return 8 * (2 * sizeof(std::uint32_t) + sizeof(std::uint8_t));
    }
}

#endif
