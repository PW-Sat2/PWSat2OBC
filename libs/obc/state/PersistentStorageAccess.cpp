#include "PersistentStorageAccess.hpp"
#include <gsl/gsl_util>

namespace obc
{
    PersistentStorageAccess::PersistentStorageAccess(drivers::spi::ISPIInterface& spi) //
        : fram(spi)
    {
    }

    void PersistentStorageAccess::Read(std::uint32_t address, gsl::span<std::uint8_t> span)
    {
        fram.Read(gsl::narrow_cast<std::uint16_t>(address), span);
    }

    void PersistentStorageAccess::Write(std::uint32_t address, gsl::span<const std::uint8_t> span)
    {
        fram.Write(gsl::narrow_cast<std::uint16_t>(address), span);
    }
}
