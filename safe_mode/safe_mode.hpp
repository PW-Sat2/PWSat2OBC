#ifndef SAFE_MODE_SAFE_MODE_HPP_
#define SAFE_MODE_SAFE_MODE_HPP_

#include "boot/settings.hpp"
#include "error_counter.hpp"
#include "fm25w/fm25w.hpp"
#include "mcu/io_map.h"
#include "msc/msc.hpp"
#include "payload/io_map.h"
#include "program_flash/boot_table.hpp"
#include "standalone/flash_driver/flash_driver.hpp"
#include "standalone/spi/spi.hpp"

class OBCSafeMode
{
  public:
    OBCSafeMode();

    BootErrorCounter errorCounting;

    drivers::msc::MCUMemoryController MCUFlash;

    StandaloneFlashDriver FlashDriver;
    program_flash::BootTable BootTable;

    SPIPeripheral SPI;
    SPIDevice<io_map::SlaveSelectFram1> Fram1Spi;
    SPIDevice<io_map::SlaveSelectFram2> Fram2Spi;
    SPIDevice<io_map::SlaveSelectFram3> Fram3Spi;

    devices::fm25w::FM25WDriver Fram1;
    devices::fm25w::FM25WDriver Fram2;
    devices::fm25w::FM25WDriver Fram3;

    std::array<devices::fm25w::IFM25WDriver*, 3> Frams;

    devices::fm25w::RedundantFM25WDriver Fram;

    boot::BootSettings BootSettings;

    std::array<std::uint8_t, 64_KB> Buffer;
};

extern OBCSafeMode SafeMode;

#endif /* SAFE_MODE_SAFE_MODE_HPP_ */
