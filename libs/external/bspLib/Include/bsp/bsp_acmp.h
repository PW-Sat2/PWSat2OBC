/***************************************************************************//**
 * @file	bsp_acmp.h
 * @brief	BSP EBI header file.
 *
 * This header file contains all the required definitions and function
 * prototypes through which to initialise the Analog Comparator (ACMP) channels
 * to detect latchup currents in the SRAM modules.
 *
 * @author	Pieter J. Botma
 * @date	04/06/2012
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

#ifndef __BSP_ACMP_H
#define __BSP_ACMP_H

#include <stdbool.h>
#include "em_acmp.h"
#include "em_cmu.h"

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

#define BSP_ACMP_SRAM1 ACMP0 ///< ACMP peripheral used to monitor SRAM1.
#define	BSP_ACMP_SRAM2 ACMP1 ///< ACMP peripheral used to monitor SRAM2.

/**
 * @brief
 *   Threshold value for ACMP0 monitoring SRAM1.
 * @details
 *   The threshold value of 10 will cause ACMP0 to interrupt if SRAM1 draws more
 *   than 52mA current.
 * @note
 *   The threshold value is determined by a combination of the following equations:
 *   \li INA139 equation (3) page 5.
 *   \li EFM32GG reference manual equation (26.1) page 672.
 */
#define BSP_ACMP_SRAM1_THRESHOLD  10

/**
 * @brief
 *   Threshold value for ACMP1 monitoring SRAM2.
 * @details
 *   The threshold value of 10 will cause ACMP1 to interrupt if SRAM2 draws more
 *   than 52mA current.
 * @note
 *   The threshold value is determined by a combination of the following equations:
 *   \li INA139 equation (3) page 5.
 *   \li EFM32GG reference manual equation (26.1) page 662.
 */
#define BSP_ACMP_SRAM2_THRESHOLD 10

#if defined(CubeCompV2B)
#define BSP_ACMP_SRAM1_CHANNEL acmpChannel0 ///< Input channel 0 for ACMP0 used to monitor SRAM1.
#define BSP_ACMP_SRAM2_CHANNEL acmpChannel0 ///< Input channel 0 for ACMP1 used to monitor SRAM2.
#elif defined(CubeCompV3) || defined(CubeCompV3B)
#define BSP_ACMP_SRAM1_CHANNEL acmpChannel2 ///< Input channel 2 for ACMP0 used to monitor SRAM1.
#define BSP_ACMP_SRAM2_CHANNEL acmpChannel0 ///< Input channel 0 for ACMP1 used to monitor SRAM2.
#endif


void BSP_ACMP_Init(ACMP_TypeDef *acmp); ///< Initialises the specified ACMP channel.

/** @} (end addtogroup ACMP) */
/** @} (end addtogroup BSP_Library) */

#endif
