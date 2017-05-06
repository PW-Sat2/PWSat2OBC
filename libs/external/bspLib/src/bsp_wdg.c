/***************************************************************************//**
 * @file	bsp_wdg.c
 * @brief	BSP WDG source file.
 *
 * This file contains all the implementations for the functions defined in \em
 * bsp_wdg.h.
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
#include "bsp_wdg.h"

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

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

bool internal; // internal enable indicator
bool external; // external enable indicator

/** @endcond */

/***************************************************************************//**
 * @author Pieter J. Botma
 * @date   15/05/2012
 *
 * This function initialises the CubeComputer's internal and external watchdog
 * modules. The internal watchdog timeout period is configured to 2 seconds,
 * and the external watchdog has a timeout period of 200ms.
 *
 * @param [in] enableInt
 *   Indicates whether the internal watchdog should be enabled or not.
 * @param [in] enableExt
 *   Indicates whether the external watchdog should be enabled or not.
 *
 ******************************************************************************/
void BSP_WDG_Init (bool enableInt, bool enableExt)
{
	// Enable clock
	CMU_ClockEnable(cmuClock_GPIO, true);

	internal = enableInt;
	external = enableExt;

	// External WDG
	if (enableExt) GPIO_PinModeSet(gpioPortF,  9, gpioModePushPull, 1);

	// Internal WDG
	WDOG_Init_TypeDef init = WDOG_INIT_DEFAULT;
	init.perSel = wdogPeriod_4k;
	init.enable = false;

	if (enableInt)
	{
		WDOG_Init(&init);
		WDOG_Enable(true);
	}
}

/***************************************************************************//**
 * @author Pieter J. Botma
 * @date   12/06/2012
 *
 * This function enables the internal watchdog.
 *
 * @note
 *   Once the internal watchdog has been enabled, it should be toggled within
 *   2 seconds continually to prevent the MCU from resetting.
 *
 ******************************************************************************/
void BSP_WDG_setInternal(bool enable)
{
	// Internal WDG
	if (internal != enable)
	{
		internal = enable;

		WDOG_Enable(enable);
	}
}

/***************************************************************************//**
 * @author Pieter J. Botma
 * @date   12/06/2012
 *
 * This function enables the external watchdog.
 *
 * @note
 *   Once the external watchdog has been enabled, it should be toggled within
 *   0.9 seconds continually to prevent the MCU from resetting.
 *
 ******************************************************************************/
void BSP_WDG_setExternal(bool enable)
{
	// External WDG
	if (external != internal)
	{
		external = internal;

		if (enable)
			GPIO_PinModeSet(gpioPortF,  9, gpioModePushPull, 1);
		else
			GPIO_PinModeSet(gpioPortF,  9, gpioModeDisabled, 0);
	}
}

/***************************************************************************//**
 * @author Pieter J. Botma
 * @date   15/05/2012
 *
 * This function toggles the the external watchdog.
 *
 * @note
 *   Toggling the watchdog while it is disabled will enable it.
 *
 ******************************************************************************/
void BSP_WDG_ToggleExt ()
{
	// Only toggle if the external watchdog has been enabled
	if(external)
		GPIO_PinOutToggle(extWDGPort,extWDGPin);
}

/** @} (end addtogroup WDG) */
/** @} (end addtogroup BSP_Library) */
