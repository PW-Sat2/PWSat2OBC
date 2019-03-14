#pragma once

#include "program_flash/boot_table.hpp"
#include "standalone/flash_driver/flash_driver.hpp"
#include "msc/msc.hpp"

void ScrubProgram(
    drivers::msc::MCUMemoryController &mcuFlash, 
    StandaloneFlashDriver &flashDriver,
    program_flash::BootTable &bootTable);