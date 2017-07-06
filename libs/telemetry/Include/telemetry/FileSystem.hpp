#ifndef LIBS_TELEMETRY_INCLUDE_TELEMETRY_FILESYSTEM_HPP_
#define LIBS_TELEMETRY_INCLUDE_TELEMETRY_FILESYSTEM_HPP_

#pragma once

#include <cstdint>
#include "base/fwd.hpp"
#include "utils.h"

namespace telemetry
{
    /**
     * @brief This type represents telemetry element related file system state.
     * @telemetry_element
     * @ingroup telemetry
     */
    class FileSystemTelemetry
    {
      public:
        /**
         * @brief
         * @param size Size of the free space area.
         */
        FileSystemTelemetry(std::uint32_t size = 0);

        /**
         * @brief Write the file system telemetry to passed buffer writer object.
         * @param[in] writer Buffer writer object that should be used to write the serialized state.
         */
        void Write(BitWriter& writer) const;

        /**
         * @brief Returns size of the file system's free space.
         * @return Size in bytes of the file system's free space.
         */
        std::uint32_t GetFreeSpace() const;

        /**
         * @brief Returns size of the serialized state in bits.
         *  @return Size of the serialized state in bits.
         */
        static constexpr std::uint32_t BitSize();

      private:
        std::uint32_t freeSpace;
    };

    inline std::uint32_t FileSystemTelemetry::GetFreeSpace() const
    {
        return this->freeSpace;
    }

    constexpr std::uint32_t FileSystemTelemetry::BitSize()
    {
        return BitLength<decltype(freeSpace)>;
    }

    static_assert(FileSystemTelemetry::BitSize() == 32, "Invalid serialized size");
}

#endif /* LIBS_TELEMETRY_INCLUDE_TELEMETRY_FILESYSTEM_HPP_ */
