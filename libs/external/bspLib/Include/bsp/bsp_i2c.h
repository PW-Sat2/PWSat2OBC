/***************************************************************************//**
 * @file	bsp_i2c.h
 * @brief	BSP I2C header file.
 *
 * This header file contains all the required definitions and function prototypes
 * through which to control the CubeComputer's External Bus Interface (EBI).
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

#ifndef __BSP_I2C_H
#define __BSP_I2C_H

#include "em_i2c.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "bsp_dma.h"


/***************************************************************************//**
 * @addtogroup BSP_Library
 * @brief Board Support Package (<b>BSP</b>) Driver Library for CubeComputer.
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup I2C
 * @brief API for CubeComputer's I2C modules.
 * @{
 ******************************************************************************/

#define BSP_I2C_SYS_ADDRESS 0xE0 ///< CubeComputer I2C slave address accessed through main I2C channel.
#define BSP_I2C_SUB_ADDRESS 0xE2 ///< CubeComputer I2C slave address accessed through subsystem I2C channel.

#define	BSP_I2C_SYS I2C0 ///< System's main I2C channel
#define	BSP_I2C_SUB I2C1 ///< Subsystem I2C channel

/// I2C transmission type indicator.
typedef enum
{
	bspI2cWrite      = I2C_FLAG_WRITE,      ///< Indicate plain write sequence: S+ADDR(W)+DATA0+P.
	bspI2cRead       = I2C_FLAG_READ,       ///< Indicate plain read sequence: S+ADDR(R)+DATA0+P.
	bspI2cWriteRead  = I2C_FLAG_WRITE_READ, ///< Indicate write sequence using two buffers: S+ADDR(W)+DATA0+DATA1+P.
	bspI2CWriteWrite = I2C_FLAG_WRITE_WRITE ///< Indicate write sequence using two buffers: S+ADDR(W)+DATA0+DATA1+P.
} BSP_I2C_ModeSelect_TydeDef;

void BSP_I2C_Init (I2C_TypeDef *i2c, bool master); 					///< Initialise the specified I2C channel.
void BSP_I2C_setSlaveMode (I2C_TypeDef *i2c, bool enable); 			///< Enable or disable slave mode for the specified I2C channel.
uint32_t BSP_I2C_masterTX (I2C_TypeDef *i2c, uint16_t address,
						BSP_I2C_ModeSelect_TydeDef flag,
						uint8_t *txBuffer, uint16_t txBufferSize,
						uint8_t *rxBuffer, uint16_t rxBufferSize); 	///< Transmit and/or receive a buffer of data over the specified I2C channel.
I2C_TransferReturn_TypeDef BSP_I2C_masterTransmit
(

		I2C_TypeDef *i2c, uint16_t address, BSP_I2C_ModeSelect_TydeDef flag,
		uint8_t *buffer1, uint16_t buffer1Size,
		uint8_t *buffer2, uint16_t buffer2Size
);
/** @} (end addtogroup I2C) */
/** @} (end addtogroup BSP_Library) */

#endif // __BSP_I2C_H
