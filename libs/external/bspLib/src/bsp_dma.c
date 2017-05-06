/***************************************************************************//**
 * @file	bsp_adc.c
 * @brief	BSP DMA source file.
 *
 * This header file contains all the required definitions and function
 * prototypes through which to setup the CubeComputer's DMA configuration.
 * @author	Pieter J. Botma
 * @date	22/05/2012
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

#include "bsp_dma.h"

/***************************************************************************//**
 * @addtogroup BSP_Library
 * @brief Board Support Package (<b>BSP</b>) Driver Library for CubeComputer.
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup DMA
 * @brief API for CubeComputer's DMA module.
 * @{
 ******************************************************************************/

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

/** DMA control block, requires proper alignment. */
#if defined (__ICCARM__)
#pragma data_alignment=256
DMA_DESCRIPTOR_TypeDef dmaControlBlock[DMA_CHAN_COUNT * 2];
#elif defined (__CC_ARM)
DMA_DESCRIPTOR_TypeDef dmaControlBlock[DMA_CHAN_COUNT * 2] __attribute__ ((aligned(256)));
#elif defined (__GNUC__)
DMA_DESCRIPTOR_TypeDef dmaControlBlock[DMA_CHAN_COUNT * 2] __attribute__ ((aligned(256)));
#else
#error Undefined toolkit, need to define alignment
#endif

DMA_CB_TypeDef cb[DMA_CHANNEL_COUNT];

/** @endcond */

/***************************************************************************//**
 * @author Pieter J. Botma
 * @date   22/05/2012
 *
 * This function configures the DMA \link DMA_Channel_TypeDef channels \endlink
 * to be used by the peripherals.
 *
 ******************************************************************************/
void BSP_DMA_Init (void)
{
	// Configure general DMA settings
	DMA_Init_TypeDef dmaInit;
	dmaInit.hprot = 0;
	dmaInit.controlBlock = dmaControlBlock;
	DMA_Init(&dmaInit);
}

/** @} (end addtogroup DMA) */
/** @} (end addtogroup BSP_Library) */
