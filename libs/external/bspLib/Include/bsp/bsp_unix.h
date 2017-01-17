/***************************************************************************//**
 * @file	bsp_unix.h
 * @brief	BSP UNIX header file.
 *
 * This header file contains all the required definitions and function
 * prototypes through which to control the CubeComputer's Unix counter.
 * @author	Pieter J. Botma
 * @date	16/05/2014
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
#ifndef BSP_UNIX_H_
#define BSP_UNIX_H_
#include <time.h>
#include "em_cmu.h"
#include "em_emu.h"
#include "em_burtc.h"
#include "em_rtc.h"
#include "em_rmu.h"
/***************************************************************************//**
 * @addtogroup BSP_Library
 * @brief Board Support Package (<b>BSP</b>) Driver Library for CubeComputer.
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup UNIX
 * @brief API for CubeComputer's Unix counter.
 * @{
 ******************************************************************************/

/* Setting up a structure to initialize the calendar
   for January 1 2012 12:00:00
   The struct tm is declared in time.h
   More information about time.h library is found on
   http://en.wikipedia.org/wiki/Time.h */
#define DEFAULT_UNIX_START_DATE                                                            \
{                                                                                          \
    0, /* tm_sec:   0 seconds (0-60, 60 = leap second)*/                                   \
    0, /* tm_min:   0 minutes (0-59) */                                                    \
   12, /* tm_hour:  0 hours (0-23) */                                                      \
    1, /* tm_mday:  1st day of the month (1 - 31) */                                       \
    0, /* tm_mon:   January (0 - 11, 0 = January) */                                       \
  114, /* tm_year:  Year 2014 (year since 1900) */                                         \
    0, /* tm_wday:  Sunday (0 - 6, 0 = Sunday) */                                          \
    0, /* tm_yday:  1st day of the year (0-365) */                                         \
   -1, /* tm_isdst: Daylight saving time; enabled (>0), disabled (=0) or unknown (<0) */   \
}

void   BSP_UNIX_Init();  ///< Initialize Unix counter.
void   BSP_UNIX_setEpochCalendar(struct tm * epochCalendar); ///< Set the Unix counter epoch (as calendar)
void   BSP_UNIX_setEpoch(time_t epoch); ///< Set the Unix counter epoch (as Unix time)
time_t BSP_UNIX_getEpoch(void); ///< Get the Unix counter epoch

/** @} (end addtogroup UNIX) */
/** @} (end addtogroup BSP_Library) */

#endif /* BSP_UNIX_H_ */
