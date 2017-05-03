#include "params.hpp"

namespace boot
{
    __attribute__((section(".boot_param.0"))) std::uint32_t Marker2;
    __attribute__((section(".boot_param.4"))) std::uint32_t Marker;
    __attribute__((section(".boot_param.2"))) std::uint32_t Marker3;
}
