#include <em_gpio.h>
#include "em_leuart.h"
#include "port_map.h"
#include "terminal.h"

uint8_t input_buffer [32] = {0};
uint32_t input_buffer_position = 0;

LEUART_Init_TypeDef leuart0Init =
{
  .enable   = leuartEnable,         /* Activate data reception on LEUn_TX pin. */
  .refFreq  = 0,                    /* Inherit the clock frequenzy from the LEUART clock source */
  .baudrate = 9600,                 /* Baudrate = 9600 bps */
  .databits = leuartDatabits8,      /* Each LEUART frame containes 8 databits */
  .parity   = leuartNoParity,       /* No parity bits in use */
  .stopbits = leuartStopbits2,      /* Setting the number of stop bits in a frame to 2 bitperiods */
};

void leuart_init(void)
{
  LEUART_Reset(LEUART0);
  LEUART_Init(LEUART0, &leuart0Init);

  /* Enable LEUART Signal Frame Interrupt */
  LEUART_IntEnable(LEUART0, LEUART_IEN_RXDATAV);

  /* Enable LEUART0 interrupt vector */
  NVIC_EnableIRQ(LEUART0_IRQn);

  LEUART0->ROUTE = LEUART_ROUTE_RXPEN |
		  	  	   LEUART_ROUTE_TXPEN |
				   LEUART_LOCATION;

  GPIO_PinModeSet(LEUART_PORT, LEUART_TX, gpioModePushPull, 1);
  GPIO_PinModeSet(LEUART_PORT, LEUART_RX, gpioModeInputPull, 1);
}

void clear_buffer()
{
	memset(input_buffer, 0, input_buffer_position);
	input_buffer_position = 0;
}

void LEUART0_IRQHandler(void) {
	uint8_t data, leuartif;

	leuartif = LEUART_IntGet(LEUART0);
	LEUART_IntClear(LEUART0, leuartif);
	data = LEUART0->RXDATA;

	if(data == 13)
	{
		terminal_handle_command(input_buffer, input_buffer_position);

		clear_buffer();
	}
	else
	{
		leuart_send_char(data);
		input_buffer[input_buffer_position++] = data;
	}
}

void leuart_send_string(uint8_t* buffer)
{
	uint8_t len = strlen(buffer);

	for(int i=0; i<len; i++)
	{
		LEUART_Tx(LEUART0, buffer[i]);
	}
}

void leuart_send_char(uint8_t c)
{
	LEUART_Tx(LEUART0, c);
}
