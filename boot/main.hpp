#ifndef BOOT_MAIN_HPP_
#define BOOT_MAIN_HPP_

#include <array>
#include "boot/settings.hpp"
#include "fm25w/fm25w.hpp"
#include "io_map.h"
#include "spi.hpp"

class OBCBootloader
{
  public:
    OBCBootloader();
    void Initialize();

    boot::BootSettings Settings;

  private:
    SPIPeripheral _spi;
    SPIDevice<io_map::SlaveSelectFram1> _fram1Spi;
    SPIDevice<io_map::SlaveSelectFram2> _fram2Spi;
    SPIDevice<io_map::SlaveSelectFram3> _fram3Spi;

    devices::fm25w::FM25WDriver _fram1;
    devices::fm25w::FM25WDriver _fram2;
    devices::fm25w::FM25WDriver _fram3;

    devices::fm25w::RedundantFM25WDriver _fram;
};

extern OBCBootloader Bootloader;

#endif /* BOOT_MAIN_HPP_ */
