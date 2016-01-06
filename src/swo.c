#include <em_system.h>
#include <core_cm3.h>
#include <em_cmu.h>
#include <em_dbg.h>
#include <stdarg.h>
#include <stdio.h>

void enableSWO(void)
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
	ITM->TER = 0x1;
}

void swoPuts(const char * str)
{
	const char* c = str;
	while (*c != '\0')
	{
		ITM_SendChar(*c);
		c++;
	}
}

void swoPrintf(const char * format, ...)
{
	va_list args;
	va_start(args, format);

	char buf[256];

	vsiprintf(buf, format, args);

	swoPuts(buf);

	va_end(args);
}
