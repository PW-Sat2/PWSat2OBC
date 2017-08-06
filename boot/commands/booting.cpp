#include <array>
#include <cstdint>
#include "boot.h"
#include "boot/params.hpp"
#include "boot/settings.hpp"
#include "bsp/bsp_boot.h"
#include "bsp/bsp_uart.h"
#include "redundancy.hpp"

using std::uint8_t;
using std::uint32_t;

void BootUpper()
{
    BSP_UART_Puts(BSP_UART_DEBUG, "\n\nBooting to upper half ");

    boot::BootReason = boot::Reason::BootToUpper;
    boot::Index = boot::BootSettings::UpperBootSlot;

    BootToAddress(BOOT_APPLICATION_BASE);
}

void SetRunlevel()
{
    BSP_UART_Puts(BSP_UART_DEBUG, "\nRunlevel: ");

    std::uint8_t runlevel = BSP_UART_rxByte(BSP_UART_DEBUG);

    if (runlevel > 3)
    {
        BSP_UART_Puts(BSP_UART_DEBUG, "Invalid runlevel\n");
        return;
    }

    boot::RequestedRunlevel = static_cast<boot::Runlevel>(runlevel);
    BSP_UART_Printf<20>(BSP_UART_DEBUG, "\nRunlevel set to %d\n", runlevel);
}

void SetClearState()
{
    BSP_UART_Puts(BSP_UART_DEBUG, "\n\nSet clear state\n");

    if (boot::ClearStateOnStartup)
    {
        BSP_UART_Puts(BSP_UART_DEBUG, "Currently: Yes\n");
    }
    else
    {
        BSP_UART_Puts(BSP_UART_DEBUG, "Currently: No\n");
    }

    BSP_UART_Puts(BSP_UART_DEBUG, "Set to [YN]: ");

    auto r = BSP_UART_rxByte(BSP_UART_DEBUG);

    switch (r)
    {
        case 'Y':
            boot::ClearStateOnStartup = true;
            BSP_UART_Puts(BSP_UART_DEBUG, "Yes\n");
            break;
        case 'N':
            boot::ClearStateOnStartup = false;
            BSP_UART_Puts(BSP_UART_DEBUG, "No\n");
            break;
        default:
            BSP_UART_Puts(BSP_UART_DEBUG, "? (Aborting)\n");
            break;
    }
}
