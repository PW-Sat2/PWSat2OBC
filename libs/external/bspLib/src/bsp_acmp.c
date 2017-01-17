/***************************************************************************//**
 * @file	bsp_acmp.c
 * @brief	BSP ACMP source file.
 *
 * This file contains all the implementations for the functions defined in \em
 * bsp_acmp.h.
 *
 * @author	Pieter J. Botma
 * @date	10/05/2012
 *******************************************************************************
 * @section License
 * <b>(C) Copyright 2021 ESL , http://http://www.esl.sun.ac.za/</b>
 *******************************************************************************
 *
 * This source code is the property of the ESL. The source and compiled code may
 * only be used on the CubeComputer.
 *
 * This copyright notice may not be removed from the source code nor changed.
 *
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: ESL has no obligation to
 * support this Software. ESL is providing the Software "AS IS", with no express
 * or implied warranties of any kind, including, but not limited to, any implied
 * warranties of merchantability or fitness for any particular purpose or
 * warranties against infringement of any proprietary rights of a third party.
 *
 * ESL will not be liable for any consequential, incidental, or special damages,
 * or any other relief, or for any claim by any third party, arising from your
 * use of this Software.
 *
 ******************************************************************************/
#include "bsp_acmp.h"

/***************************************************************************//**
 * @addtogroup BSP_Library
 * @brief Board Support Package (<b>BSP</b>) Driver Library for CubeComputer.
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup ACMP
 * @brief API for CubeComputer ACMP channels.
 * @{
 ******************************************************************************/

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

void InitSRAM1(void)
{
	CMU_ClockEnable(cmuClock_ACMP0, true);

	ACMP_Init_TypeDef init = ACMP_INIT_DEFAULT;

	init.fullBias = true;
	init.biasProg = 0xf;
	init.hysteresisLevel = acmpHysteresisLevel0;
	init.interruptOnRisingEdge = true;
	init.vddLevel = BSP_ACMP_SRAM1_THRESHOLD;

	// Init and set ACMP channel
	ACMP_Reset(ACMP0);
	ACMP_Init(ACMP0, &init);
	ACMP_ChannelSet(ACMP0, acmpChannelVDD, BSP_ACMP_SRAM1_CHANNEL);

	// Enable edge interrupt
	ACMP_IntEnable(ACMP0, ACMP_IEN_EDGE);

	// Wait for warmup
	while (!(ACMP0->STATUS & ACMP_STATUS_ACMPACT));

	ACMP_IntClear(ACMP0, ACMP_IFC_EDGE);

	// Enable interrupts
	NVIC_ClearPendingIRQ(ACMP0_IRQn);
	NVIC_EnableIRQ(ACMP0_IRQn);
}

void InitSRAM2 (void)
{
	CMU_ClockEnable(cmuClock_ACMP1, true);

	ACMP_Init_TypeDef init = ACMP_INIT_DEFAULT;

	init.fullBias = true;
	init.biasProg = 0xf;
	init.hysteresisLevel = acmpHysteresisLevel0;
	init.interruptOnRisingEdge = true;
	init.vddLevel = BSP_ACMP_SRAM2_THRESHOLD;

	// Init and set ACMP channel
	ACMP_Reset(ACMP1);
	ACMP_Init(ACMP1, &init);
	ACMP_ChannelSet(ACMP1, acmpChannelVDD, BSP_ACMP_SRAM2_CHANNEL);

	ACMP_IntEnable(ACMP1, ACMP_IEN_EDGE);   // Enable edge interrupt

	// Wait for warmup
	while (!(ACMP1->STATUS & ACMP_STATUS_ACMPACT)) ;

	ACMP_IntClear(ACMP1, ACMP_IFC_EDGE);

	// Enable interrupts
	NVIC_ClearPendingIRQ(ACMP0_IRQn);
	NVIC_EnableIRQ(ACMP0_IRQn);
}

/** @endcond */

/***************************************************************************//**
 * @author Pieter J. Botma
 * @date   28/03/2012
 *
 * This function initialises the specified ACMP channel to monitor the current
 * passing through a current sensor on the relevant SRAM supply line. If a
 * threshold value is surpassed, the ACMP will generate an interrupt which must
 * be handled (by ACMP0_IRQn()) by the main operating program.
 *
 * @param[in] acmp
 *   Pointer to ACMP channel to be initialised.
 ******************************************************************************/
void BSP_ACMP_Init(ACMP_TypeDef *acmp)
{
	// Initialise the specified ACMP channel
	if(acmp == BSP_ACMP_SRAM1)
	  InitSRAM1();
	else if (acmp == BSP_ACMP_SRAM2)
	  InitSRAM2();
}

/** @} (end addtogroup ACMP) */
/** @} (end addtogroup BSP_Library) */
