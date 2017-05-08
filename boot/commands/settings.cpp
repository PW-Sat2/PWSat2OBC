#include "bsp/bsp_boot.h"
#include "bsp/bsp_uart.h"

void SetBootIndex()
{
    BSP_UART_Puts(BSP_UART_DEBUG, "\n\nNew Boot Index: ");

    // get boot index
    auto index = USART_Rx(BSP_UART_DEBUG);

    BSP_UART_Printf<4>(BSP_UART_DEBUG, "%d", index);

    // test boot index
    if (index > BOOT_TABLE_SIZE)
    {
        BSP_UART_Puts(BSP_UART_DEBUG, "\n\nError: Boot index out of bounds!");
        return;
    }

    // upload new program to internal flash
    BOOT_setBootIndex(index);
    BOOT_resetBootCounter();

    BSP_UART_Puts(BSP_UART_DEBUG, "...Done!");
}
