/*
 * terminal.c
 *
 *  Created on: 15 mar 2016
 *      Author: terianil
 */

#include <em_gpio.h>
#include "em_leuart.h"
#include "em_dma.h"


/* Defining the LEUART1 initialization data */
LEUART_Init_TypeDef leuart0Init =
{
  .enable   = leuartEnableTx,       /* Activate data reception on LEUn_TX pin. */
  .refFreq  = 0,                    /* Inherit the clock frequenzy from the LEUART clock source */
  .baudrate = 9600,                 /* Baudrate = 9600 bps */
  .databits = leuartDatabits8,      /* Each LEUART frame containes 8 databits */
  .parity   = leuartNoParity,       /* No parity bits in use */
  .stopbits = leuartStopbits2,      /* Setting the number of stop bits in a frame to 2 bitperiods */
};

void initLeuart(void)
{
  LEUART_Reset(LEUART0);
  LEUART_Init(LEUART0, &leuart0Init);

  LEUART0->ROUTE = LEUART_ROUTE_TXPEN |
                   LEUART_ROUTE_LOCATION_LOC0;

  /* Enable GPIO for LEUART1. TX is on D4 */
  GPIO_PinModeSet(gpioPortD,                /* GPIO port */
                  4,                        /* GPIO port number */
                  gpioModePushPull,         /* Pin mode is set to push pull */
                  1);                       /* High idle state */

  LEUART_Tx(LEUART0, 'a');
}
