#include "bsp/bsp_boot.h"
#include "bsp/bsp_uart.h"

void PrintBootTable()
{
    char debugStr[80] = {0};

    auto debugLen = sprintf((char*)debugStr, "\n\nBoot Table Entries:\n");
    BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);

    for (decltype(BOOT_TABLE_SIZE) entry = 1; entry <= BOOT_TABLE_SIZE; entry++)
    {
        debugLen = sprintf((char*)debugStr, "\n%d. ", entry);
        BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);

        // check if entry is valid
        if (*(uint8_t*)(BOOT_TABLE_BASE + BOOT_getOffsetValid(entry)) != BOOT_ENTRY_ISVALID)
        {
            debugLen = sprintf((char*)debugStr, "Not Valid!");
            BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);

            continue;
        }

        // display description
        for (decltype(BOOT_ENTRY_DESCRIPTION_SIZE) i = 0; i < BOOT_ENTRY_DESCRIPTION_SIZE; i++)
        {
            auto data = (uint8_t*)(BOOT_TABLE_BASE + BOOT_getOffsetDescription(entry) + i);

            if (*data == '\r' || *data == '\n')
            {
                break;
            }

            USART_Tx(BSP_UART_DEBUG, *data);
        }
    }
}
