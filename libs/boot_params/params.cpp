#include "params.hpp"

namespace boot
{
    __attribute__((section(".boot_param.0"))) decltype(MagicNumber) MagicNumber;
    __attribute__((section(".boot_param.1"))) decltype(BootReason) BootReason;
    __attribute__((section(".boot_param.2"))) decltype(Index) Index;
}
