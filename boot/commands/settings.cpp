#include "bsp/bsp_boot.h"
#include "bsp/bsp_uart.h"

void SetBootIndex()
{
    char debugStr[80];

    auto debugLen = sprintf((char*)debugStr, "\n\nNew Boot Index: ");
    BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);

    // get boot index
    auto index = USART_Rx(BSP_UART_DEBUG);

    debugLen = sprintf((char*)debugStr, "%d", index);
    BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);

    // test boot index
    if (index > BOOT_TABLE_SIZE)
    {
        debugLen = sprintf((char*)debugStr, "\n\nError: Boot index out of bounds!");
        BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
        return;
    }

    // upload new program to internal flash
    BOOT_setBootIndex(index);
    BOOT_resetBootCounter();

    debugLen = sprintf((char*)debugStr, "...Done!");
    BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
}
