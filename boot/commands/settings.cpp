#include <bitset>
#include "bsp/bsp_boot.h"
#include "bsp/bsp_uart.h"
#include "main.hpp"

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

static void PrintBootSlots(std::uint8_t slots)
{
    std::bitset<6> bits(slots & 0b00111111);

    for (auto i = 0; i < 6; i++)
    {
        if (bits[i])
        {
            BSP_UART_Printf<2>(BSP_UART_DEBUG, "%d ", i);
        }
    }
}

void ShowBootSettings()
{
    BSP_UART_Puts(BSP_UART_DEBUG, "\nBoot settings:");

    auto counter = Bootloader.Settings.BootCounter();
    auto bootSlots = Bootloader.Settings.BootSlots();
    auto failsafeBootSlots = Bootloader.Settings.FailsafeBootSlots();

    BSP_UART_Puts(BSP_UART_DEBUG, "\nBoot slots: ");
    PrintBootSlots(bootSlots);

    BSP_UART_Puts(BSP_UART_DEBUG, "\nFailsafe boot slots: ");
    PrintBootSlots(failsafeBootSlots);

    BSP_UART_Printf<40>(BSP_UART_DEBUG, "\nBoot counter: %d", counter);
    BSP_UART_Puts(BSP_UART_DEBUG, "\n");
}
