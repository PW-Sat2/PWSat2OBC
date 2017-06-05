#include "base/crc.h"
#include "bsp/bsp_boot.h"
#include "bsp/bsp_uart.h"
#include "main.hpp"
#include "program_flash/boot_table.hpp"
#include "xmodem.h"

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
}
