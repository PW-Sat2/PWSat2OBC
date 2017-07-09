#include "safe_mode.hpp"

#include "mcu/io_map.h"

OBCSafeMode::OBCSafeMode()
    : FlashDriver(io_map::ProgramFlash::FlashBase), //
      BootTable(FlashDriver),                       //
      Fram1Spi(SPI),                                //
      Fram2Spi(SPI),                                //
      Fram3Spi(SPI),                                //
      Fram1(Fram1Spi),                              //
      Fram2(Fram2Spi),                              //
      Fram3(Fram3Spi),                              //
      Frams{&Fram1, &Fram2, &Fram3},                //
      Fram(errorCounting, Frams),                   //
      BootSettings(Fram)
{
    SPI.Initialize();
}
