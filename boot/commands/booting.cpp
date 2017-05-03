#include <cstdint>
#include "boot.h"
#include "boot/params.hpp"
#include "bsp/bsp_boot.h"
#include "bsp/bsp_uart.h"

using std::uint8_t;

void BootUpper()
{
    BSP_UART_Puts(BSP_UART_DEBUG, "\n\nBooting to upper half ");

    boot::BootReason = boot::Reason::BootToUpper;
    boot::Index = 8;

    BootToAddress(BOOT_APPLICATION_BASE);
}

void BootToSelected()
{
    auto index = USART_Rx(BSP_UART_DEBUG);
    if (index > BOOT_TABLE_SIZE)
    {
        BSP_UART_Puts(BSP_UART_DEBUG, "\n\nError: Boot index out of bounds!");
        return;
    }

    BSP_UART_Printf<60>(BSP_UART_DEBUG, "\nBooting index: %d", index);

    boot::BootReason = boot::Reason::SelectedIndex;

    BootToAddress(LoadApplication(index));
}
