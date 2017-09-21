#include "bsp/bsp_boot.h"
#include "bsp/bsp_uart.h"
#include "main.hpp"

void PrintBootTable()
{
    BSP_UART_Puts(BSP_UART_DEBUG, "\n\nBoot Table Entries:\n");

    for (auto entry = 0; entry < program_flash::BootTable::EntriesCount; entry++)
    {
        BSP_UART_Printf<50>(BSP_UART_DEBUG, "\n%d. ", entry);

        auto e = Bootloader.BootTable.Entry(entry);

        if (!e.IsValid())
        {
            BSP_UART_Puts(BSP_UART_DEBUG, "Not Valid!");

            continue;
        }

        BSP_UART_Puts(BSP_UART_DEBUG, e.Description());

        BSP_UART_Printf<50>(BSP_UART_DEBUG, " (CRC: %.4X Size: %ld bytes)", e.Crc(), e.Length());
    }

    BSP_UART_Puts(BSP_UART_DEBUG, "\n");
}

void EraseBootTable()
{
    BSP_UART_Puts(BSP_UART_DEBUG, "\nErasing program flash....");
    auto result = lld_ChipEraseOp(reinterpret_cast<FLASHDATA*>(BOOT_TABLE_BASE));
    BSP_UART_Printf<50>(BSP_UART_DEBUG, "\nDone (%d)", result);
}
