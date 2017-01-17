/***************************************************************************//**
 * @file	bsp_rtc.h
 * @brief	BSP RTC header file.
 *
 * This header file contains all the required definitions and function
 * prototypes through which to control the CubeComputer's Real Time Clock (RTC).
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

#ifndef __BSP_RTC_H
#define __BSP_RTC_H

#include "em_cmu.h"
#include "em_rtc.h"

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

void     BSP_RTC_Init (); ///< Initialise RTC.
void     BSP_RTC_tick (); ///< Increment RTC  in milliseconds.
void     BSP_RTC_setTime (uint32_t time); ///< Set the time of the RTC.
uint32_t BSP_RTC_getTimeSec(); ///< Return the current seconds counter of the RTC.
uint32_t BSP_RTC_getTimeMSec(); ///< Return the current milliseconds counter of the RTC.

/** @} (end addtogroup RTC) */
/** @} (end addtogroup BSP_Library) */

#endif // __BSP_RTC_H
