#include "bsp/bsp_boot.h"
#include "bsp/bsp_uart.h"
#include "crc.h"
#include "xmodem.h"

void UploadApplication()
{
    char debugStr[80] = {0};
    auto debugLen = sprintf((char*)debugStr, "\n\nBoot Index: ");
    BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);

    // get boot index
    auto index = USART_Rx(BSP_UART_DEBUG);

    debugLen = sprintf((char*)debugStr, "%d", index);
    BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);

    // test boot index
    if (index == 0x00)
    {
        debugLen = sprintf((char*)debugStr, "\nError: Cant override safe mode program!");
        BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
        return;
    }
    if (index > BOOT_TABLE_SIZE)
    {
        debugLen = sprintf((char*)debugStr, "\nError: Boot index out of bounds!");
        BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
        return;
    }

    debugLen = sprintf((char*)debugStr, "\nUpload Binary: ");
    BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);

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

    debugLen = sprintf((char*)debugStr, "\nBoot Description: ");
    BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);

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

    debugLen = sprintf((char*)debugStr, "...Done!");
    BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
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
