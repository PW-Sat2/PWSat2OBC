#include <em_gpio.h>
#include "em_leuart.h"
#include "em_dma.h"
#include "port_map.h"

LEUART_Init_TypeDef leuart0Init =
{
  .enable   = leuartEnable,       /* Activate data reception on LEUn_TX pin. */
  .refFreq  = 0,                    /* Inherit the clock frequenzy from the LEUART clock source */
  .baudrate = 9600,                 /* Baudrate = 9600 bps */
  .databits = leuartDatabits8,      /* Each LEUART frame containes 8 databits */
  .parity   = leuartNoParity,       /* No parity bits in use */
  .stopbits = leuartStopbits2,      /* Setting the number of stop bits in a frame to 2 bitperiods */
};

void LEUART0_IRQHandler(void) {
	uint32_t data, leuartif;

	leuartif = LEUART_IntGet(LEUART0);
	LEUART_IntClear(LEUART0, leuartif);
	data = LEUART0->RXDATA;

	LEUART_Tx(LEUART0, data);
}

void initLeuart(void)
{
  LEUART_Reset(LEUART0);
  LEUART_Init(LEUART0, &leuart0Init);

  /* Enable LEUART Signal Frame Interrupt */
  LEUART_IntEnable(LEUART0, LEUART_IEN_RXDATAV);

  /* Enable LEUART0 interrupt vector */
  NVIC_EnableIRQ(LEUART0_IRQn);

  LEUART0->ROUTE = LEUART_ROUTE_RXPEN |
		  	  	   LEUART_ROUTE_TXPEN |
                   LEUART_ROUTE_LOCATION_LOC0;

  GPIO_PinModeSet(LEUART_PORT, LEUART_TX,                        /* GPIO port number */
                  gpioModePushPull,         /* Pin mode is set to push pull */
                  1);                       /* High idle state */

  GPIO_PinModeSet(LEUART_PORT, LEUART_RX,                        /* GPIO port number */
                    gpioModeInputPull,         /* Pin mode is set to push pull */
                    1);
}
