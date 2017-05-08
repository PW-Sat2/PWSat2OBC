#include "bsp/bsp_boot.h"
#include "bsp/bsp_uart.h"

void PrintBootTable()
{
    BSP_UART_Puts(BSP_UART_DEBUG, "\n\nBoot Table Entries:\n");

    for (decltype(BOOT_TABLE_SIZE) entry = 1; entry <= BOOT_TABLE_SIZE; entry++)
    {
        BSP_UART_Printf<5>(BSP_UART_DEBUG, "\n%d. ", entry);

        // check if entry is valid
        if (*(uint8_t*)(BOOT_TABLE_BASE + BOOT_getOffsetValid(entry)) != BOOT_ENTRY_ISVALID)
        {
            BSP_UART_Puts(BSP_UART_DEBUG, "Not Valid!");

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

        auto crc = BOOT_getCRC(entry);
        auto size = BOOT_getLen(entry);
        BSP_UART_Printf<50>(BSP_UART_DEBUG, " (CRC: %.4X Size: %ld bytes)", crc, size);
    }
}
