/***************************************************************************/ /**
  * @file
  * @brief   CubeComputer Test Program Main.
  * @author  Pieter Botma
  * @version 2.0
  ******************************************************************************/
#include "boot.h"
#include "includes.h"

#define COMMS_TIMEOUT 5000 // milliseconds

void waitForComms(uint32_t timeoutTicks_ms);
uint8_t verifyBootIndex(uint8_t bootIndex);
uint8_t verifyBootCounter(void);
uint8_t verifyApplicationCRC(uint8_t entryIndex);
void resetPeripherals(void);
void restClocks(void);

/***************************************************************************/ /**
  * @brief  Main function
  * Main is called from _program_start, see assembly startup file
  ******************************************************************************/
int main(void)
{
    uint8_t bootIndex;
    uint32_t bootAddress;

    //    BOOT_DownloadResult_Typedef downloadError;

    CHIP_Init();

    // set up general clocks
    CMU_OscillatorEnable(cmuOsc_HFRCO, true, true);
    CMU_OscillatorEnable(cmuOsc_LFRCO, true, true);
    CMU_HFRCOBandSet(cmuHFRCOBand_14MHz);
    CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFRCO);

    if (SysTick_Config(CMU_ClockFreqGet(cmuClock_CORE) / 1000)) // Setup SysTick Timer for 1 msec interrupts
    {
        while (1)
            ;
    }

    BSP_DMA_Init();
    COMMS_Init();
    BSP_EBI_Init();
    BSP_EBI_disableSRAM(bspEbiSram1);
    BSP_EBI_disableSRAM(bspEbiSram2);
    Delay(5);
    BSP_EBI_enableSRAM(bspEbiSram1);
    BSP_EBI_enableSRAM(bspEbiSram2);

    debugLen = sprintf((char*)debugStr, "\n\nBootloader (b = boot, s = set, x = XMODEM upload, l = list entries, r = reset):");
    BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);

    waitForComms(COMMS_TIMEOUT);

    debugLen = sprintf((char*)debugStr, "\nTimeout exceeded - booting");
    BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);

    bootIndex = BOOT_getBootIndex();

    if (bootIndex == 0)
    {
        debugLen = sprintf((char*)debugStr, "\n\nSafe Mode boot index... Booting safe mode!");
        BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
    }
    else if (!verifyBootIndex(bootIndex))
    {
        bootIndex = 0;
        BOOT_setBootIndex(0);

        debugLen = sprintf((char*)debugStr, "\n\nInvalid boot index... Booting safe mode!");
        BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
    }
    else if (!verifyBootCounter())
    {
        bootIndex = 0;
        BOOT_setBootIndex(0);

        debugLen = sprintf((char*)debugStr, "\n\nBoot counter expired... Booting safe mode!");
        BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
    }
    else
    {
        BOOT_decBootCounter();
    }

    bootAddress = LoadApplication(bootIndex);
    BootToAddress(bootAddress);
}
void waitForComms(uint32_t timeoutTicks_ms)
{
    // Setup SysTick Timer for 1 msec interrupts
    SysTick_Config(CMU_ClockFreqGet(cmuClock_CORE) / 1000);

    msTicks = 0;

    BSP_UART_txByte(BSP_UART_DEBUG, '#');

    do
    {
        COMMS_processMsg();
        EMU_EnterEM1();
    } while ((msTicks < timeoutTicks_ms) || uartReceived);

    SysTick->CTRL &= (~SysTick_CTRL_ENABLE_Msk); // disable SysTick timer
}

uint8_t verifyBootIndex(uint8_t bootIndex)
{
    return ((bootIndex > 0) && (bootIndex <= BOOT_TABLE_SIZE));
}

uint8_t verifyBootCounter(void)
{
    return (BOOT_getBootCounter() > 0);
}
