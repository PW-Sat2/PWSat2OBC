#ifndef LIBS_STATE_ISTORAGE_ACCESS_HPP
#define LIBS_STATE_ISTORAGE_ACCESS_HPP

#pragma once

#include <cstdint>
#include "gsl/span"

namespace obc
{
    struct IStorageAccess
    {
        virtual void Read(std::uint32_t address, gsl::span<std::uint8_t> span) = 0;

        virtual void Write(std::uint32_t address, gsl::span<const std::uint8_t> span) = 0;
    };
}
#endif
