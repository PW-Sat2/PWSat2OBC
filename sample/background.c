/*
 * background.c
 *
 *  Created on: 11 Jul 2013
 *      Author: pjbotma
 */

#include "background.h"

uint32_t resetCause = 0;

/***************************************************************************//**
 * @brief
 *   Displays reset cause over debug UART and returns it.
 ******************************************************************************/
uint32_t BACKGROUND_displayResetCause(void)
{
	if((resetCause&RMU_RSTCAUSE_PORST))
	{
		debugLen = sprintf((char*)debugStr,"\n\nPORST: Power on reset");
	}else if((resetCause&RMU_RSTCAUSE_BODUNREGRST))
	{
		debugLen = sprintf((char*)debugStr,"\n\nBOD: Unregulated power brown-out reset");
	}else if((resetCause&RMU_RSTCAUSE_BODREGRST))
	{
		debugLen = sprintf((char*)debugStr,"\n\nBODREG: Regulated power brown-out reset");
	}else if((resetCause&(RMU_RSTCAUSE_EXTRST | RMU_RSTCAUSE_WDOGRST)) > 0)
	{
		debugLen = sprintf((char*)debugStr,"\n\nEXTWDOG: External reset or watchdog reset");
	}else if((resetCause&RMU_RSTCAUSE_EXTRST))
	{
		debugLen = sprintf((char*)debugStr,"\n\nEXTRST: External reset");
	}else if((resetCause&RMU_RSTCAUSE_WDOGRST))
	{
		debugLen = sprintf((char*)debugStr,"\n\nWDOGRST: Watchdog reset");
	}else if((resetCause&(RMU_RSTCAUSE_LOCKUPRST | RMU_RSTCAUSE_SYSREQRST)))
	{
		debugLen = sprintf((char*)debugStr,"\n\nLOCKSYS: Lockup reset or System reset request");
	}else if((resetCause&RMU_RSTCAUSE_LOCKUPRST))
	{
		debugLen = sprintf((char*)debugStr,"\n\nLOCKRST: Lockup reset");
	}else if((resetCause&RMU_RSTCAUSE_SYSREQRST))
	{
		debugLen = sprintf((char*)debugStr,"\n\nSYSRST: System reset request");
	}else
	{
		debugLen = sprintf((char*)debugStr,"\n\nUnknown reset cause");
	}

	BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)debugStr,debugLen, true);
	return resetCause;
}

/***************************************************************************//**
 * @brief
 *   Returns reset cause.
 ******************************************************************************/
uint32_t BACKGROUND_getResetCause(void)
{
	resetCause = RMU_ResetCauseGet();

	RMU_ResetCauseClear();

	return resetCause;
}

void BACKGROUND_displayWelcomeMessage(void)
{
#if defined(CubeCompV3)
	debugLen = sprintf((char*)debugStr,"\n\nCubeComputerV3A ");
#elif defined(CubeCompV3B)
	debugLen = sprintf((char*)debugStr,"\n\nCubeComputerV3B ");
#endif
	BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)debugStr,debugLen, true);

#if defined(Base)
	debugLen = sprintf((char*)debugStr,"Base build:\n\n't' = RTC/Runtime test\n'a' = ADC test\n'e' = EBI test(EEPROM, Flash, SRAM)\n'm' = MicroSD test\n'1' = Enable SRAM1\n'!' = Disable SRAM1\n'2' = Enable SRAM2\n'@' = Disable SRAM2\n'l' = SRAM status\n'w' = Test Internal Watchdog\n'W' = Test External Watchdog\n'c' = command list\n'r' = Reset MCU\n's' = Test SRAM");
#endif

#if defined(Debug)
	debugLen = sprintf((char*)debugStr,"Debug build:\n\n't' = RTC/Runtime test\n'a' = ADC test\n'e' = EBI test(EEPROM, Flash, SRAM)\n'm' = MicroSD test\n'1' = Enable SRAM1\n'!' = Disable SRAM1\n'2' = Enable SRAM2\n'@' = Disable SRAM2\n'l' = SRAM status\n'w' = Test Internal Watchdog\n'W' = Test External Watchdog\n'c' = command list\n'r' = Reset MCU\n's' = Test SRAM");
#endif

#if defined(Release)
	debugLen = sprintf((char*)debugStr,"Release build:\n\n't' = RTC/Runtime test\n'a' = ADC test\n'e' = EBI test(EEPROM, Flash, SRAM)\n'm' = MicroSD test\n'w' = Test Internal Watchdog\n'W' = Test External Watchdog\n'c' = command list\n'r' = Reset MCU");
#endif

	BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)debugStr,debugLen, true);
}
