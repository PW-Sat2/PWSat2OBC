#ifndef SAFE_MODE_SAFE_MODE_HPP_
#define SAFE_MODE_SAFE_MODE_HPP_

#include "msc/msc.hpp"
#include "program_flash/boot_table.hpp"
#include "standalone/flash_driver/flash_driver.hpp"

class OBCSafeMode
{
  public:
    OBCSafeMode();

    drivers::msc::MCUMemoryController MCUFlash;

    StandaloneFlashDriver FlashDriver;
    program_flash::BootTable BootTable;
};

extern OBCSafeMode SafeMode;

#endif /* SAFE_MODE_SAFE_MODE_HPP_ */
