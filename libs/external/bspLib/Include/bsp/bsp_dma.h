/***************************************************************************//**
 * @file	bsp_dma.h
 * @brief	BSP DMA header file.
 *
 * This header file contains all the required definitions and function
 * prototypes through which to setup the CubeComputer's DMA configuration.
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

#ifndef __DMACTRL_H
#define __DMACTRL_H

#include "em_dma.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * @addtogroup BSP_Library
 * @brief Board Support Package (<b>BSP</b>) Driver Library for CubeComputer.
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup DMA
 * @brief API for CubeComputer's DMA configuration.
 * @{
 ******************************************************************************/

/// DMA channels for different CubeComputer peripherals.
typedef enum
{
	DMA_CHANNEL_ADC_SCAN = 0, ///< ADC scan channel.
	DMA_CHANNEL_ADC_SNGL = 1, ///< ADC single sample channel.
	DMA_CHANNEL_DEBUG_TX = 2, ///< UART transmit channel.
	DMA_CHANNEL_DEBUG_RX = 3, ///< UART receive channel.
	DMA_CHANNEL_MSC      = 4, ///< Internal flash channel
	DMA_CHANNEL_I2C_TX   = 5, ///< I2C Transmit channel
	DMA_CHANNEL_I2C_RX   = 6, ///< I2C Transmit channel
	DMA_CHANNEL_COUNT    = 7  ///< Total number of channels.
} DMA_Channel_TypeDef;

extern DMA_DESCRIPTOR_TypeDef dmaControlBlock[]; ///< DMA descriptor configuration block.
extern DMA_CB_TypeDef cb[]; ///< DMA callback function configuration block.

void BSP_DMA_Init (void); ///< Initialises DMA to be used on the CubeComputer.

/** @} (end addtogroup ADC) */
/** @} (end addtogroup BSP_Library) */

#ifdef __cplusplus
}
#endif

#endif /* __DMACTRL_H */
