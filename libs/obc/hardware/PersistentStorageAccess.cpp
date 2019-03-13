#include "PersistentStorageAccess.hpp"
#include <gsl/gsl_util>

namespace obc
{
    PersistentStorageAccess::PersistentStorageAccess(
        std::array<drivers::spi::ISPIInterface*, 3> spis)
        :                             //
          _fm25wDrivers{              //
              {*spis[0]},             //
              {*spis[1]},             //
              {*spis[2]}},            //
          _driver{
              {&_fm25wDrivers[0],     //
                  &_fm25wDrivers[1],  //
                  &_fm25wDrivers[2]}} //
    {
    }

    void PersistentStorageAccess::Read(std::uint32_t address, gsl::span<std::uint8_t> span)
    {
        _driver.Read(gsl::narrow_cast<std::uint16_t>(address), span);
    }

    void PersistentStorageAccess::Write(std::uint32_t address, gsl::span<const std::uint8_t> span)
    {
        _driver.Write(gsl::narrow_cast<std::uint16_t>(address), span);
    }

    devices::fm25w::RedundantFM25WDriver& PersistentStorageAccess::GetRedundantDriver()
    {
        return _driver;
    }
}
