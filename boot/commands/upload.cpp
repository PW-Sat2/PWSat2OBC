#include "base/crc.h"
#include "base/gcc_workaround.hpp"
#include "bsp/bsp_boot.h"
#include "bsp/bsp_uart.h"
#include "main.hpp"
#include "program_flash/boot_table.hpp"
#include "xmodem.h"

using program_flash::FlashStatus;

void UploadApplication()
{
    BSP_UART_Puts(BSP_UART_DEBUG, "\n\nBoot Index: ");

    // get boot index
    auto index = USART_Rx(BSP_UART_DEBUG);

    BSP_UART_txByte(BSP_UART_DEBUG, index);

    if (index < '0' || index - '0' >= program_flash::BootTable::EntriesCount)
    {
        BSP_UART_Puts(BSP_UART_DEBUG, "\nError: Boot index out of bounds!");
        return;
    }

    BSP_UART_Puts(BSP_UART_DEBUG, "\nUpload Binary: ");

    index = index - '0';

    auto entry = Bootloader.BootTable.Entry(index);

    auto len = XMODEM_upload(&entry);

    entry.Length(len);

    entry.Crc(entry.CalculateCrc());

    // mark entry as valid
    BOOT_setValid(index);

    BSP_UART_Puts(BSP_UART_DEBUG, "\nBoot Description: ");

    uint8_t desc[BOOT_ENTRY_DESCRIPTION_SIZE] = {0};

    // get description
    for (decltype(BOOT_ENTRY_DESCRIPTION_SIZE) i = 0; i < BOOT_ENTRY_DESCRIPTION_SIZE; i++)
    {
        desc[i] = USART_Rx(BSP_UART_DEBUG);

        USART_Tx(BSP_UART_DEBUG, desc[i]);

        if (desc[i] == '\n')
        {
            break;
        }
    }

    entry.Description(reinterpret_cast<char*>(desc));

    BSP_UART_Puts(BSP_UART_DEBUG, "...Done!");
}

void UploadSafeMode()
{
    XMODEM_upload(nullptr);
    BSP_UART_Puts(BSP_UART_DEBUG, "...Done!");
}

void CopyBootloader()
{
    BSP_UART_Puts(BSP_UART_DEBUG, "\nCopying current bootloader to external flash....\n");

    gsl::span<std::uint8_t> sourceSpan(reinterpret_cast<std::uint8_t*>(1), 64_KB - 1);

    for (std::uint8_t i = 0; i < program_flash::BootTable::BootloaderCopies; i++)
    {
        BSP_UART_Printf<10>(BSP_UART_DEBUG, "\tCopy %d:\t", i);

        auto copy = Bootloader.BootTable.GetBootloaderCopy(i);

        BSP_UART_Puts(BSP_UART_DEBUG, "Erasing\t");
        auto r = copy.Erase();
        if (r != FlashStatus::NotBusy)
        {
            BSP_UART_Printf<10>(BSP_UART_DEBUG, "Failed %d\n", num(r));
            return;
        }

        BSP_UART_Puts(BSP_UART_DEBUG, "Programing\t");
        r = copy.Write(0, gcc_workaround::ReadByte0());
        if (r != FlashStatus::NotBusy)
        {
            BSP_UART_Printf<10>(BSP_UART_DEBUG, "Failed %d\n", num(r));
            return;
        }

        r = copy.Write(1, sourceSpan);
        if (r != FlashStatus::NotBusy)
        {
            BSP_UART_Printf<10>(BSP_UART_DEBUG, "Failed %d\n", num(r));
            return;
        }

        BSP_UART_Puts(BSP_UART_DEBUG, "\n");
    }

    BSP_UART_Puts(BSP_UART_DEBUG, "Copy done\n");
}

void CopySafeMode()
{
    BSP_UART_Puts(BSP_UART_DEBUG, "\nCopying current safe mode to external flash....\n");

    gsl::span<std::uint8_t> sourceSpan(reinterpret_cast<std::uint8_t*>(BOOT_SAFEMODE_BASE_CODE), program_flash::SafeModeCopy::Size);

    for (std::uint8_t i = 0; i < program_flash::BootTable::SafeModeCopies; i++)
    {
        BSP_UART_Printf<10>(BSP_UART_DEBUG, "\tCopy %d:\t", i);

        auto copy = Bootloader.BootTable.GetSafeModeCopy(i);

        BSP_UART_Puts(BSP_UART_DEBUG, "Erasing\t");
        auto r = copy.Erase();
        if (r != FlashStatus::NotBusy)
        {
            BSP_UART_Printf<10>(BSP_UART_DEBUG, "Failed %d\n", num(r));
            return;
        }

        BSP_UART_Puts(BSP_UART_DEBUG, "Programing\t");

        r = copy.Write(0, sourceSpan);
        if (r != FlashStatus::NotBusy)
        {
            BSP_UART_Printf<10>(BSP_UART_DEBUG, "Failed %d\n", num(r));
            return;
        }

        BSP_UART_Puts(BSP_UART_DEBUG, "\n");
    }

    BSP_UART_Puts(BSP_UART_DEBUG, "Copy done\n");
}
