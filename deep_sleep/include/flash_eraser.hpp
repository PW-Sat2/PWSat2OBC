#pragma once

#include "n25q/n25q.h"
#include "mcu/io_map.h"
#include "payload/io_map.h"
#include "standalone/spi/spi.hpp"

class FlashEraser
{
  public:
    FlashEraser(SPIPeripheral& spi);

    void Initialize();

    void Run();

  private:
    SPIDevice<io_map::SlaveSelectFlash1> _flash1Spi;
    SPIDevice<io_map::SlaveSelectFlash2> _flash2Spi;
    SPIDevice<io_map::SlaveSelectFlash3> _flash3Spi;
    devices::n25q::N25QDriver _flash1;
    devices::n25q::N25QDriver _flash2;
    devices::n25q::N25QDriver _flash3;
};
