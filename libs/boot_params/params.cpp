#include "params.hpp"

namespace boot
{
    __attribute__((section(".boot_param.0"))) std::uint32_t MagicNumber;
    __attribute__((section(".boot_param.1"))) Reason BootReason;
}
