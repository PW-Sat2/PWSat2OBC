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
      BootSettings(Fram),                           //
      Flash1Spi(SPI),                               //
      Flash2Spi(SPI),                               //
      Flash3Spi(SPI),                               //
      Flash1(errorCounting, 1, Flash1Spi),          //
      Flash2(errorCounting, 1, Flash2Spi),          //
      Flash3(errorCounting, 1, Flash3Spi)
{
    SPI.Initialize();

    Fram1Spi.Initialize();
    Fram2Spi.Initialize();
    Fram3Spi.Initialize();

    Flash1Spi.Initialize();
    Flash2Spi.Initialize();
    Flash3Spi.Initialize();

    Flash1.Reset();
    Flash2.Reset();
    Flash3.Reset();
}
