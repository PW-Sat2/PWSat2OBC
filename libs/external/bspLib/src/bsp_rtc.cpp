/***************************************************************************//**
 * @file	bsp_rtc.c
 * @brief	BSP RTC source file.
 *
 * This file contains all the implementations for the functions defined in \em
 * bsp_rtc.h.
 * @author	Pieter J. Botma
 * @date	14/05/2012
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

#include "bsp_rtc.h"

/***************************************************************************//**
 * @addtogroup BSP_Library
 * @brief Board Support Package (<b>BSP</b>) Driver Library for CubeComputer.
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup RTC
 * @brief API for CubeComputer's real time clock (RTC).
 * @{
 ******************************************************************************/

//uint32_t msec; ///< Milliseconds counter
//uint32_t sec;  ///< Seconds counter

/***************************************************************************//**
 * @author Pieter J. Botma
 * @date   14/05/2012
 *
 * This function initialises the CubeComputer's real time clock to create an
 * interrupt roughly every millisecond, or 32 ticks of 32.768KHz crystal.
 *
 ******************************************************************************/
void BSP_RTC_Init ()
{
	//sec = 0;
	//msec = 0;

	// RTC clock settings
	CMU_ClockEnable(cmuClock_RTC, true);

	// RTC counter settings
	uint32_t ticks = CMU_ClockFreqGet(cmuClock_RTC); // number of ticks for increment interval
	RTC_CompareSet(0, ticks);

	// RTC interrupt settings
	RTC_IntEnable(RTC_IEN_COMP0);
	NVIC_EnableIRQ(RTC_IRQn);

	// RTC settings
	RTC_Init_TypeDef init = RTC_INIT_DEFAULT;
	RTC_Init(&init);
}

/***************************************************************************//**
 * @author Pieter J. Botma
 * @date   14/05/2012
 *
 * This function increments the milliseconds counter of the RTC. If 1024 ticks
 * have been reached (1024*32 = 32768 = LFXO), the seconds counter is
 * incremented and the milliseconds counter is reset.
 *
 ******************************************************************************/
/*void BSP_RTC_tick ()
{
	msec++;

	if (msec >= 1024)
	{
		msec = 0;
		sec++;
	}
}*/

/***************************************************************************//**
 * @author Pieter J. Botma
 * @date   14/05/2012
 *
 * This functions sets the seconds counter equal to the specified value,
 * \b time, and resets the milliseconds counter.
 *
 * @param [in] time
 * 	The new value of the seconds counter.
 ******************************************************************************/
/*void BSP_RTC_setTime (uint32_t time)
{
	sec = time;
	msec = 0;
}*/

/***************************************************************************//**
 * @author Pieter J. Botma
 * @date   14/05/2012
 *
 * This function return the value of the seconds counter of the RTC.
 *
 * @return
 * 	Returns the value of the seconds counter.
 *
 ******************************************************************************/
/*uint32_t BSP_RTC_getTimeSec()
{
	return sec;
}*/

/***************************************************************************//**
 * @author Pieter J. Botma
 * @date   14/05/2012
 *
 * This function return the value of the milliseconds counter of the RTC.
 *
 * @return
 * 	Returns the value of the milliseconds counter.
 *
 ******************************************************************************/
/*uint32_t BSP_RTC_getTimeMSec()
{
	return msec;
}*/

/** @} (end addtogroup RTC) */
/** @} (end addtogroup BSP_Library) */
