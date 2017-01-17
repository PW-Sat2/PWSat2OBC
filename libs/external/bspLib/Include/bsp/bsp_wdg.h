/***************************************************************************//**
 * @file	bsp_wdg.h
 * @brief	BSP WDG header file.
 *
 * This header file contains all the required definitions and function
 * prototypes through which to control the CubeComputer's Watchdog (WDG) modules.
 * @author	Pieter J. Botma
 * @date	15/05/2012
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
#ifndef __BSP_WDG_H
#define __BSP_WDG_H
#include <stdbool.h>
#include "em_wdog.h"
#include "em_gpio.h"
#include "em_cmu.h"
/***************************************************************************//**
 * @addtogroup BSP_Library
 * @brief Board Support Package (<b>BSP</b>) Driver Library for CubeComputer.
 * @{
 ******************************************************************************/
/***************************************************************************//**
 * @addtogroup WDG
 * @brief API for CubeComputer's watchdog (WDG) modules.
 * @{
 ******************************************************************************/
#define extWDGPort gpioPortF ///< Define external watchdog MCU port.
#define extWDGPin          9 ///< Define external watchdog MCU pin.
#define BSP_WDG_ToggleInt()	WDOG_Feed() ///< Define internal watchdog toggle function.

// Global functions
void BSP_WDG_Init (bool enableInt, bool enableExt); ///< Initialise internal and external watchdog modules.
void BSP_WDG_setInternal(bool enable); /// Enables the internal watchdog
void BSP_WDG_setExternal(bool enable); /// Enables the external watchdog
void BSP_WDG_ToggleExt (); ///< Toggle external watchdog.
/** @} (end addtogroup WDG) */
/** @} (end addtogroup BSP_Library) */
#endif // __BSP_WDG_H
