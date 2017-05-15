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
         */
        SystemStartup(std::uint32_t counter, std::uint8_t index);

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
         * @brief Read the system startup telemetry element subsystem state from passed reader.
         * @param[in] reader Buffer reader that should be used to read the serialized state.
         */
        void Read(Reader& reader);

        /**
         * @brief Write the system startup telemetry element to passed buffer writer object.
         * @param[in] writer Buffer writer object that should be used to write the serialized state.
         */
        void Write(Writer& writer) const;

        /**
         * @brief Returns size of the serialized state in bytes.
         * @return Size of the serialized state in bytes.
         */
        static constexpr std::uint32_t Size();

        /**
         * @brief Reports when two system startup objects are different.
         *
         * @remark Used by Telemetry container.
         * @param[in] state Object to compare to.
         * @return True whether the two objects are different, false otherwise.
         */
        bool IsDifferent(const SystemStartup& state) const;

      private:
        /**
         * @brief Current boot counter value.
         */
        std::uint32_t bootCounter;

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

    constexpr std::uint32_t SystemStartup::Size()
    {
        return sizeof(std::uint32_t) + sizeof(std::uint8_t);
    }

    inline bool SystemStartup::IsDifferent(const SystemStartup& state) const
    {
        return this->bootCounter != state.bootCounter || this->bootIndex != state.bootIndex;
    }
}

#endif
