#include "boot.h"
#include <em_usart.h>
#include "bsp/bsp_boot.h"
#include "bsp/bsp_uart.h"

static void resetPeripherals(void)
{
    SysTick->CTRL &= (~SysTick_CTRL_ENABLE_Msk);

    MSC_Deinit();
    DMA_Reset();
    USART_Reset(BSP_UART_DEBUG);
}

static void resetClocks(void)
{
    CMU->HFCORECLKEN0 &= ~CMU_HFCORECLKEN0_DMA;
    CMU->HFPERCLKEN0 &= ~CMU_HFPERCLKEN0_UART1;
}

static uint8_t verifyApplicationCRC(uint8_t entryIndex)
{
    uint8_t *startAddr, *endAddr;
    uint16_t expectedCRC, actualCRC;

    startAddr = (uint8_t*)(BOOT_APPLICATION_BASE);
    endAddr = (uint8_t*)(startAddr + BOOT_getLen(entryIndex));

    actualCRC = BOOT_calcCRC(startAddr, endAddr);
    expectedCRC = BOOT_getCRC(entryIndex);

    return (actualCRC == expectedCRC);
}

void BootToAddress(uint32_t baseAddress)
{
    resetPeripherals();
    resetClocks();

    BOOT_boot(baseAddress);

    while (1)
        ;
}

uint32_t LoadApplication(uint8_t bootIndex)
{
    size_t debugLen;
    uint8_t debugStr[256];

    if (bootIndex == 0)
    {
        return BOOT_SAFEMODE_BASE_CODE;
    }

    if (verifyApplicationCRC(bootIndex))
    {
        debugLen = sprintf((char*)debugStr, "\n\nBooting application!");
        BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);

        return BOOT_APPLICATION_BASE;
    }

    BOOT_DownloadResult_Typedef downloadError = BOOT_tryDownloadEntryToApplicationSpace(bootIndex);

    if (downloadError)
    {
        bootIndex = 0;
        BOOT_setBootIndex(bootIndex);

        debugLen = sprintf((char*)debugStr, "\n\nUnable to load application (Error: %d)... Booting safe mode!", downloadError);
        BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);

        return BOOT_SAFEMODE_BASE_CODE;
    }
    else
    {
        debugLen = sprintf((char*)debugStr, "\n\nBooting application!");
        BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);

        return BOOT_APPLICATION_BASE;
    }
}
