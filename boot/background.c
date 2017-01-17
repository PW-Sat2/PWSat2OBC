/*
 * background.c
 *
 *  Created on: 11 Jul 2013
 *      Author: pjbotma
 */

#include "includes.h"

volatile uint32_t  sec = 0; ///< total time in seconds
volatile uint16_t msec = 0; ///< total time in mseconds

volatile uint32_t singleErrors = 0;
volatile uint32_t doubleErrors = 0;
volatile uint32_t multiErrors  = 0;

/**************************************************************************//**
 * @brief RTC_IRQHandler
 * Interrupt Service Routine for general purpose input/output pins
 *****************************************************************************/
void GPIO_EVEN_IRQHandler(void)
{
	uint8_t errors;

	errors = ( (uint8_t)GPIO_PortInGet(gpioPortB) ) & 0x5;

	switch(errors)
	{
	case 0x0:
		multiErrors++;
		break;

	case 0x1:
		doubleErrors++;
		break;

	case 0x4:
		singleErrors++;
		break;
	}

	GPIO_IntClear(_GPIO_IFC_MASK & 0x5);
}
