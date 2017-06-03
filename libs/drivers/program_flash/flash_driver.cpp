#include "flash_driver.hpp"
#include <chrono>
#include "base/os.h"
#include "lld.h"
#include "logger/logger.h"

using namespace std::chrono_literals;

namespace program_flash
{
    FlashSpan::FlashSpan(IFlashDriver& flash, std::size_t offset) : _flash(flash), _offset(offset)
    {
    }
}
