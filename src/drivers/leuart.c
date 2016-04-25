#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include <em_cmu.h>
#include <em_gpio.h>
#include <em_leuart.h>

#include "io_map.h"

QueueHandle_t leuart0sink;

void leuartInit(xQueueHandle sink)
{
	LEUART_Init_TypeDef leuart0Init =
			{
					.enable = leuartEnable, /* Activate data reception on LEUn_TX pin. */
					.refFreq = 0, /* Inherit the clock frequenzy from the LEUART clock source */
					.baudrate = LEUART0_BAUDRATE, /* Baudrate = 9600 bps */
					.databits = leuartDatabits8, /* Each LEUART frame containes 8 databits */
					.parity = leuartNoParity, /* No parity bits in use */
					.stopbits = leuartStopbits2, /* Setting the number of stop bits in a frame to 2 bitperiods */
			};

	leuart0sink = sink;

	CMU_ClockEnable(cmuClock_CORELE, true);
	CMU_ClockEnable(cmuClock_LEUART0, true);

	LEUART_Reset(LEUART0);
	LEUART_Init(LEUART0, &leuart0Init);

	/* Enable LEUART Signal Frame Interrupt */
	LEUART_IntEnable(LEUART0, LEUART_IEN_RXDATAV);

	/* Enable LEUART0 interrupt vector */
	NVIC_EnableIRQ(LEUART0_IRQn);

	LEUART0->ROUTE = LEUART_ROUTE_RXPEN |
	LEUART_ROUTE_TXPEN |
	LEUART0_LOCATION;

	GPIO_PinModeSet(LEUART0_PORT, LEUART0_TX, gpioModePushPull, 1);
	GPIO_PinModeSet(LEUART0_PORT, LEUART0_RX, gpioModeInputPull, 1);
}

void leuartPuts(const char* buffer)
{
	const size_t len = strlen(buffer);

	for (size_t i = 0; i < len; i++)
	{
		LEUART_Tx(LEUART0, buffer[i]);
	}
}

void leuartPrintf(const char * text, ...)
{
	char buf[255] = { 0 };

	va_list args;
	va_start(args, text);

	vsniprintf(buf, sizeof(buf), text, args);

	leuartPuts(buf);

	va_end(args);
}

void leuartPutc(uint8_t c)
{
	LEUART_Tx(LEUART0, c);
}

void LEUART0_IRQHandler(void)
{
	uint8_t data = LEUART_RxDataGet(LEUART0);

	BaseType_t woken = pdFALSE;
	xQueueSendToFrontFromISR(leuart0sink, &data, &woken);

	portEND_SWITCHING_ISR(woken);
}
