#ifndef LIBS_STATE_BOOT_STATE_HPP
#define LIBS_STATE_BOOT_STATE_HPP

#pragma once

#include <cstdint>
#include "base/fwd.hpp"

namespace state
{
    /**
     * @brief This type represents system's persistent program startup state.
     * @persistent_state
     */
    class BootState final
    {
      public:
        /**
         * @brief ctor.
         */
        BootState();

        /**
         * @brief ctor.
         * @param[in] value New reboot counter value.
         */
        BootState(std::uint32_t value);

        /**
         * @brief Returns current reboot counter value.
         * @return Current reboot counter value.
         */
        std::uint32_t BootCounter() const noexcept;

        /**
         * @brief Read the program startup state from passed reader.
         * @param[in] reader Buffer reader that should be used to read the serialized state.
         */
        void Read(Reader& reader);

        /**
         * @brief Write the program startup state to passed buffer writer object.
         * @param[in] writer Buffer writer object that should be used to write the serialized state.
         */
        void Write(Writer& writer) const;

        /**
         * @brief Returns size of the serialized state in bytes.
         * @return Size of the serialized state in bytes.
         */
        static constexpr std::uint32_t Size();

      private:
        std::uint32_t bootCounter;
    };

    inline std::uint32_t BootState::BootCounter() const noexcept
    {
        return this->bootCounter;
    }

    constexpr std::uint32_t BootState::Size()
    {
        return sizeof(std::uint32_t);
    }
}

#endif
