#include "safe_mode.hpp"

#include "mcu/io_map.h"

OBCSafeMode::OBCSafeMode() : FlashDriver(io_map::ProgramFlash::FlashBase), BootTable(FlashDriver)
{
}
