#include <bitset>
#include "bsp/bsp_boot.h"
#include "bsp/bsp_uart.h"
#include "main.hpp"
#include "utils.h"

static Option<std::uint8_t> ReadBootSlots()
{
    std::bitset<6> result(0);

    for (auto i = 0; i < 3; i++)
    {
        BSP_UART_Printf<20>(BSP_UART_DEBUG, "\tSlot %d: ", i);
        auto index = USART_Rx(BSP_UART_DEBUG);

        BSP_UART_txByte(BSP_UART_DEBUG, index);

        if (index < '0' || (index - '0') >= program_flash::BootTable::EntriesCount)
        {
            BSP_UART_Puts(BSP_UART_DEBUG, "\tInvalid boot index\n");
            return None<std::uint8_t>();
        }

        result[index - '0'] = true;
    }

    if (result.count() != 3)
    {
        BSP_UART_Puts(BSP_UART_DEBUG, "\t3 slots must be selected\n");
        return None<std::uint8_t>();
    }

    return Some(static_cast<std::uint8_t>(result.to_ulong()));
}

void SetBootIndex()
{
    BSP_UART_Puts(BSP_UART_DEBUG, "\n\nNew Boot slots (Primary):\n");

    auto bootSlots = ReadBootSlots();

    if (!bootSlots.HasValue)
    {
        return;
    }

    BSP_UART_Puts(BSP_UART_DEBUG, "\n\nNew Boot slots (failsafe):\n");

    auto failsafeBootSlots = ReadBootSlots();

    if (!failsafeBootSlots.HasValue)
    {
        return;
    }

    Bootloader.Settings.MarkAsValid();

    if (!Bootloader.Settings.BootSlots(bootSlots.Value))
    {
        BSP_UART_Puts(BSP_UART_DEBUG, "Failed to set boot slots\n");
        return;
    }

    if (!Bootloader.Settings.FailsafeBootSlots(failsafeBootSlots.Value))
    {
        BSP_UART_Puts(BSP_UART_DEBUG, "Failed to set failsafe boot slots\n");
        return;
    }

    Bootloader.Settings.BootCounter(boot::BootSettings::DefaultBootCounter);
    Bootloader.Settings.ConfirmBoot();

    BSP_UART_Puts(BSP_UART_DEBUG, "\nNew boot slots set\n");
}

static void PrintBootSlots(std::uint8_t slots)
{
    if (slots == boot::BootSettings::SafeModeBootSlot)
    {
        BSP_UART_Puts(BSP_UART_DEBUG, "Safe Mode");
        return;
    }

    if (slots == boot::BootSettings::UpperBootSlot)
    {
        BSP_UART_Puts(BSP_UART_DEBUG, "Upper");
        return;
    }

    std::bitset<program_flash::BootTable::EntriesCount> bits(slots);

    for (auto i = 0; i < program_flash::BootTable::EntriesCount; i++)
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
    auto counterLastConfirmed = Bootloader.Settings.LastConfirmedBootCounter();
    auto bootSlots = Bootloader.Settings.BootSlots();
    auto failsafeBootSlots = Bootloader.Settings.FailsafeBootSlots();

    BSP_UART_Puts(BSP_UART_DEBUG, "\nBoot slots: ");
    PrintBootSlots(bootSlots);

    BSP_UART_Puts(BSP_UART_DEBUG, "\nFailsafe boot slots: ");
    PrintBootSlots(failsafeBootSlots);

    BSP_UART_Printf<40>(BSP_UART_DEBUG, "\nBoot counter: %ld", counter);
    BSP_UART_Printf<40>(BSP_UART_DEBUG, "\nLast confirmed boot counter: %ld", counterLastConfirmed);

    BSP_UART_Puts(BSP_UART_DEBUG, "\n");
}

void SetBootSlotToSafeMode()
{
    Bootloader.Settings.BootSlots(boot::BootSettings::SafeModeBootSlot);
    Bootloader.Settings.MarkAsValid();
    BSP_UART_Puts(BSP_UART_DEBUG, "\nPrimary boot slot set to safe-mode\n");
}

void SetBootSlotToUpper()
{
    Bootloader.Settings.BootSlots(boot::BootSettings::UpperBootSlot);
    Bootloader.Settings.MarkAsValid();
    BSP_UART_Puts(BSP_UART_DEBUG, "\nPrimary boot slot set to upper\n");
}
