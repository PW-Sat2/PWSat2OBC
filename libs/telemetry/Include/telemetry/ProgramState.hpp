#ifndef LIBS_TELEMETRY_INCLUDE_TELEMETRY_PROGRAMSTATE_HPP_
#define LIBS_TELEMETRY_INCLUDE_TELEMETRY_PROGRAMSTATE_HPP_

#pragma once

#include <cstdint>
#include "base/fwd.hpp"

namespace telemetry
{
    /**
     * @brief This type represents telemetry element related to
     * state of the currently executed program.
     * @telemetry_element
     * @ingroup telemetry
     */
    class ProgramState final
    {
      public:
        /**
         * @brief TimeState telemetry unique identifier.
         */
        static constexpr int Id = 2;

        /**
         * @brief .ctor
         */
        constexpr ProgramState();

        /**
         * @brief .ctor
         * @param[in] crc CRC-16 code of currently executed program.
         */
        constexpr ProgramState(std::uint16_t crc);

        /**
         * @brief Returns current program CRC-16 value.
         * @return Current program's CRC-16.
         */
        constexpr std::uint16_t Crc() const noexcept;

        /**
         * @brief Write the program state telemetry to passed buffer writer object.
         * @param[in] writer Buffer writer object that should be used to write the serialized state.
         */
        void Write(BitWriter& writer) const;

        /**
         * @brief Returns size of the serialized state in bits.
         * @return Size of the serialized state in bits.
         */
        constexpr static std::uint32_t BitSize();

      private:
        std::uint16_t programCrc;
    };

    constexpr ProgramState::ProgramState() : programCrc(0)
    {
    }

    constexpr ProgramState::ProgramState(std::uint16_t crc) : programCrc(crc)
    {
    }

    constexpr std::uint16_t ProgramState::Crc() const noexcept
    {
        return this->programCrc;
    }

    constexpr std::uint32_t ProgramState::BitSize()
    {
        return 8 * sizeof(std::uint16_t);
    }
}

#endif /* LIBS_TELEMETRY_INCLUDE_TELEMETRY_PROGRAMSTATE_HPP_ */
