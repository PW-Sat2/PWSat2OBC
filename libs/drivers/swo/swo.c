#include <em_system.h>
#include <core_cm3.h>
#include <em_cmu.h>
#include <em_dbg.h>
#include <stdarg.h>
#include <stdio.h>
#include "swo.h"

// Extenseion to the original platform.
uint32_t ITM_SendCharToChannel (uint32_t ch, uint8_t channel);

void SwoEnable(void)
{
		/* Enable GPIO clock. */
		CMU->HFPERCLKEN0 |= CMU_HFPERCLKEN0_GPIO;

		/* Enable Serial wire output pin */
		GPIO->ROUTE |= GPIO_ROUTE_SWOPEN;

		/* Set location 0 */
		GPIO->ROUTE = (GPIO->ROUTE & ~(_GPIO_ROUTE_SWLOCATION_MASK)) | GPIO_ROUTE_SWLOCATION_LOC0;

		/* Enable output on pin - GPIO Port F, Pin 2 */
		GPIO->P[5].MODEL &= ~(_GPIO_P_MODEL_MODE2_MASK);
		GPIO->P[5].MODEL |= GPIO_P_MODEL_MODE2_PUSHPULL;

		/* Enable debug clock AUXHFRCO */
		CMU->OSCENCMD = CMU_OSCENCMD_AUXHFRCOEN;

	/* Wait until clock is ready */
	while (!(CMU->STATUS & CMU_STATUS_AUXHFRCORDY))
		;

	/* Enable trace in core debug */
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	ITM->LAR = 0xC5ACCE55; // unlock (magic number)
	ITM->TER = 0x0;
	ITM->TCR = 0x0;
	TPI->SPPR = 2;
	TPI->ACPR = 0xf;
	ITM->TPR = 0x0;
	DWT->CTRL = 0x400003FF;
	ITM->TCR = 0x0001000D;
	TPI->FFCR = 0x00000100;
	ITM->TER = 0x7f;
}

void SwoPutsOnChannel(uint8_t channel, const char * str)
{
	const char* c = str;
	while (*c != '\0')
	{
		ITM_SendCharToChannel(*c, channel);
		c++;
	}
}

void SwoPrintfOnChannel(uint8_t channel, const char * format, ...)
{
	va_list args;
	va_start(args, format);
	SwoVPrintfOnChannel(channel, format, args);
	va_end(args);
}

void SwoVPrintfOnChannel(uint8_t channel, const char * format, va_list arguments)
{
	char buffer[256];
	vsniprintf(buffer, sizeof(buffer), format, arguments);
	SwoPutsOnChannel(channel, buffer);
}
