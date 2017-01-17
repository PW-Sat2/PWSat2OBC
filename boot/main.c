/***************************************************************************//**
 * @file
 * @brief   CubeComputer Test Program Main.
 * @author  Pieter Botma
 * @version 2.0
 ******************************************************************************/
#include "includes.h"

#define COMMS_TIMEOUT 5000 // milliseconds

void waitForComms(uint32_t timeoutTicks_ms);
uint8_t verifyBootIndex(uint8_t bootIndex);
uint8_t verifyBootCounter(void);
uint8_t verifyApplicationCRC(uint8_t entryIndex);
void resetPeripherals(void);
void restClocks(void);

/***************************************************************************//**
 * @brief  Main function
 * Main is called from _program_start, see assembly startup file
 ******************************************************************************/
int main(void)
{
	uint8_t bootIndex;
	uint32_t bootAddress;
	BOOT_DownloadResult_Typedef downloadError;

	CHIP_Init();

	// set up general clocks
	CMU_OscillatorEnable(cmuOsc_HFRCO, true, true);
	CMU_OscillatorEnable(cmuOsc_LFRCO, true, true);
	CMU_HFRCOBandSet(cmuHFRCOBand_28MHz);
	CMU_ClockSelectSet(cmuClock_HF,  cmuSelect_HFRCO);

	if (SysTick_Config(CMU_ClockFreqGet(cmuClock_CORE) / 1000))	// Setup SysTick Timer for 1 msec interrupts
	{
		while (1) ;
	}

	BSP_DMA_Init();
	COMMS_Init();
	BSP_EBI_Init();
	BSP_EBI_disableSRAM(bspEbiSram1);
	BSP_EBI_disableSRAM(bspEbiSram2);
	Delay(5);
	BSP_EBI_enableSRAM(bspEbiSram1);
	BSP_EBI_enableSRAM(bspEbiSram2);

	debugLen = sprintf((char*)debugStr,"\n\nBootloader (b = boot, s = set, x = XMODEM upload, l = list entries, r = reset):");
	BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)debugStr,debugLen, true);

	waitForComms(COMMS_TIMEOUT);

	bootIndex = BOOT_getBootIndex();

	if(bootIndex == 0)
	{
		debugLen = sprintf((char*)debugStr,"\n\nSafe Mode boot index... Booting safe mode!");
		BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)debugStr,debugLen, true);

		goto boot;
	}

	if(!verifyBootIndex(bootIndex))
	{
		bootIndex = 0;
		BOOT_setBootIndex(bootIndex);

		debugLen = sprintf((char*)debugStr,"\n\nInvalid boot index... Booting safe mode!");
		BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)debugStr,debugLen, true);

		goto boot;
	}

	if(!verifyBootCounter())
	{
		bootIndex = 0;
		BOOT_setBootIndex(bootIndex);

		debugLen = sprintf((char*)debugStr,"\n\nBoot counter expired... Booting safe mode!");
		BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)debugStr,debugLen, true);

		goto boot;
	}

	BOOT_decBootCounter();

	if(verifyApplicationCRC(bootIndex))
	{
		debugLen = sprintf((char*)debugStr,"\n\nBooting application!");
		BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)debugStr,debugLen, true);

		goto boot;
	}

	downloadError = BOOT_tryDownloadEntryToApplicationSpace(bootIndex);

	if(downloadError)
	{
		bootIndex = 0;
		BOOT_setBootIndex(bootIndex);

		debugLen = sprintf((char*)debugStr,"\n\nUnable to load application (Error: %d)... Booting safe mode!", downloadError);
		BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)debugStr,debugLen, true);

		goto boot;
	}
	else
	{
		debugLen = sprintf((char*)debugStr,"\n\nBooting application!");
		BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)debugStr,debugLen, true);
	}

boot:

	resetPeripherals();
	restClocks();

	bootAddress = (bootIndex == 0) ? BOOT_SAFEMODE_BASE_CODE : BOOT_APPLICATION_BASE;
	BOOT_boot(bootAddress);

	while(1);
}

void waitForComms(uint32_t timeoutTicks_ms)
{
	// Setup SysTick Timer for 1 msec interrupts
	SysTick_Config(CMU_ClockFreqGet(cmuClock_CORE) / 1000);

	msTicks = 0;

	do{
		COMMS_processMsg();
		EMU_EnterEM1();
	}
	while ((msTicks < timeoutTicks_ms) || uartReceived);

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

uint8_t verifyApplicationCRC(uint8_t entryIndex)
{
	uint8_t *startAddr, *endAddr;
	uint16_t expectedCRC, actualCRC;

	startAddr = (uint8_t*)( BOOT_APPLICATION_BASE );
	endAddr   = (uint8_t*)( startAddr + BOOT_getLen(entryIndex) );

	actualCRC   = BOOT_calcCRC(startAddr, endAddr);
	expectedCRC = BOOT_getCRC(entryIndex);

	return (actualCRC == expectedCRC);
}

void resetPeripherals(void)
{
	MSC_Deinit();
	DMA_Reset();
	USART_Reset(BSP_UART_DEBUG);
}

void restClocks(void)
{
	CMU->HFCORECLKEN0 &= ~CMU_HFCORECLKEN0_DMA;
	CMU->HFPERCLKEN0 &= ~CMU_HFPERCLKEN0_UART1;
}
