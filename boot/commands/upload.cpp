#include "base/crc.h"
#include "bsp/bsp_boot.h"
#include "bsp/bsp_uart.h"
#include "xmodem.h"

void UploadApplication()
{
    BSP_UART_Puts(BSP_UART_DEBUG, "\n\nBoot Index: ");

    // get boot index
    auto index = USART_Rx(BSP_UART_DEBUG);

    BSP_UART_Printf<4>(BSP_UART_DEBUG, "%d", index);

    // test boot index
    if (index == 0x00)
    {
        BSP_UART_Puts(BSP_UART_DEBUG, "\nError: Cant override safe mode program!");
        return;
    }
    if (index > BOOT_TABLE_SIZE)
    {
        BSP_UART_Puts(BSP_UART_DEBUG, "\nError: Boot index out of bounds!");
        return;
    }

    BSP_UART_Puts(BSP_UART_DEBUG, "\nUpload Binary: ");

    auto len = XMODEM_upload(index);

    // update entry length
    BOOT_setLen(index, len);

    // calculate actual CRC of downloaded application
    auto crcStart = (uint8_t*)(BOOT_TABLE_BASE + BOOT_getOffsetProgram(index));
    auto crcEnd = crcStart + len;
    auto crcVal = CRC_calc(crcStart, crcEnd);

    // update entry crc
    BOOT_setCRC(index, crcVal);

    // mark entry as valid
    BOOT_setValid(index);

    BSP_UART_Puts(BSP_UART_DEBUG, "\nBoot Description: ");

    uint8_t desc[BOOT_ENTRY_DESCRIPTION_SIZE];

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

    // update entry description
    BOOT_programDescription(index, desc);

    // change boot index
    BOOT_setBootIndex(index);
    BOOT_resetBootCounter();

    BSP_UART_Puts(BSP_UART_DEBUG, "...Done!");
}

void UploadSafeMode()
{
    // get boot index
    auto index = 0;

    // upload safe mode application
    XMODEM_upload(index);

    // change boot index
    BOOT_setBootIndex(index);
    BOOT_resetBootCounter();
}
