#ifndef LIBS_OBC_STATE_ISTORAGE_ACCESS_HPP
#define LIBS_OBC_STATE_ISTORAGE_ACCESS_HPP

#pragma once

#include <cstdint>
#include "gsl/span"

/**
 * @addtogroup StateDef
 * @{
 */
namespace obc
{
    /**
     * @brief Interface of external memory controller.
     *
     * Objects that implement this interface are supposed to provide sequential access
     * to non volatile memory.
     */
    struct IStorageAccess
    {
        /**
         * @brief Reads requested part of the memory to provided buffer.
         * @param[in] address Address to read from.
         * @param[in] span View of the data buffer that should receive the read data.
         */
        virtual void Read(std::uint32_t address, gsl::span<std::uint8_t> span) = 0;

        /**
         * @brief Writes contents of passed buffer to the requested part of memory.
         * @param[in] address Address to write to.
         * @param[in] span View of a buffer whose contents should be written.
         */
        virtual void Write(std::uint32_t address, gsl::span<const std::uint8_t> span) = 0;
    };
}

/** @} */

#endif
