/***************************************************************************//**
 * @file	bsp_i2c.c
 * @brief	BSP EBI source file.
 *
 * This file contains all the implementations for the functions defined in \em
 * bsp_i2c.h.
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
#include "bsp_i2c.h"
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
/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
void InitSys (bool master)
{
	// Setup clocks
	CMU_ClockEnable(cmuClock_HFPER, true);
	CMU_ClockEnable(cmuClock_GPIO, true);
	CMU_ClockEnable(cmuClock_I2C0, true);

	// Using PC6 (SDA) and PC7 (SCL)
	GPIO_PinModeSet(gpioPortC, 7, gpioModeWiredAndFilter, 1);
	GPIO_PinModeSet(gpioPortC, 6, gpioModeWiredAndFilter, 1);

	// Enable pins at location 2
	I2C0->ROUTE = I2C_ROUTE_SDAPEN |
			I2C_ROUTE_SCLPEN |
			I2C_ROUTE_LOCATION_LOC2;

	// Use default settings, defined in em_i2c.h
	I2C_Init_TypeDef i2cInit = I2C_INIT_DEFAULT;

	i2cInit.master = (master) ? 1 : 0; // enable/disable master mode

	// Initializing the I2C
	I2C_Init(I2C0, &i2cInit);

	// Abort if undefined reset
	I2C0->CMD = I2C_CMD_ABORT;

	if(!master) // i.e. slave
	{
		// Setting up to enable slave mode
		I2C0->SADDR = BSP_I2C_SYS_ADDRESS;
		I2C0->SADDRMASK = _I2C_SADDRMASK_MASK;
		I2C0->CTRL |= I2C_CTRL_SLAVE | I2C_CTRL_AUTOACK;

		// enable interrupts for slave mode
		I2C0->IFC = ~0;
		I2C0->IEN = I2C_IEN_ADDR | I2C_IEN_RXDATAV | I2C_IEN_ACK | I2C_IEN_NACK | I2C_IEN_SSTOP;

		// Clear and enable interrupt from I2C module
		NVIC_ClearPendingIRQ(I2C0_IRQn);
		NVIC_EnableIRQ(I2C0_IRQn);
	}
}

void InitSub (bool master)
{
	// Setup clocks
	CMU_ClockEnable(cmuClock_GPIO, true);
	CMU_ClockEnable(cmuClock_I2C1, true);

	// Using PC4 (SDA) and PC5 (SCL)
	GPIO_PinModeSet(gpioPortC, 4, gpioModeWiredAndFilter, 1);
	GPIO_PinModeSet(gpioPortC, 5, gpioModeWiredAndFilter, 1);

	// Enable pins at location 2
	I2C1->ROUTE = I2C_ROUTE_SDAPEN |
			I2C_ROUTE_SCLPEN |
			I2C_ROUTE_LOCATION_LOC0;

	// Use default settings, defined in em_i2c.h
	I2C_Init_TypeDef i2cInit = I2C_INIT_DEFAULT;

	i2cInit.master = (master) ? 1 : 0; // enable/disable master mode

	// Initializing the I2C
	I2C_Init(I2C1, &i2cInit);

	// Abort if undefined reset
	I2C1->CMD = I2C_CMD_ABORT;

	if(!master) // i.e. slave
	{
		// Setting up to enable slave mode
		I2C1->SADDR = BSP_I2C_SUB_ADDRESS;
		I2C1->SADDRMASK = _I2C_SADDRMASK_MASK;
		I2C1->CTRL |= I2C_CTRL_SLAVE | I2C_CTRL_AUTOACK;

		// enable interrupts for slave mode
		I2C1->IFC = ~0;
		I2C1->IEN = I2C_IEN_ADDR | I2C_IEN_RXDATAV | I2C_IEN_ACK | I2C_IEN_NACK | I2C_IEN_SSTOP;

		// Clear and enable interrupt from I2C module
		NVIC_ClearPendingIRQ(I2C1_IRQn);
		NVIC_EnableIRQ(I2C1_IRQn);
	}
}
/** @endcond */
/***************************************************************************//**
 * @author Pieter J. Botma
 * @date   23/05/2012
 *
 * This function initialises the specified I2C channel as a master transmitters.
 * @param[in] i2c
 *   Pointer of the I2C module to be initialised.
 * @param[in] master
 *   Determines if specified I2C module should be initialized as master or slave
 *
 ******************************************************************************/
void BSP_I2C_Init (I2C_TypeDef *i2c, bool master)
{
	if(i2c == BSP_I2C_SYS)
		InitSys(master); // Initialise main I2C channel
	else if (i2c == BSP_I2C_SUB)
		InitSub(master); // Initialise subsystem I2C channel
}

/***************************************************************************//**
 * @author Pieter J. Botma
 * @date   23/05/2012
 *
 * This function enables or disables the interrupts needed for slave mode
 * operation of the specified I2C module.
 *
 * @param[in] i2c
 *   Pointer to the specified I2C module.
 * @param[in] enable
 *   Indicates if slave mode should be enabled or disabled for the specified I2C.
 ******************************************************************************/
void BSP_I2C_setSlaveMode(I2C_TypeDef *i2c, bool enable)
{
	if(enable)
	{
		// Clear flags + set and enable interrupts
		i2c->IFC = ~0;
		i2c->IEN = I2C_IEN_ADDR | I2C_IEN_RXDATAV | I2C_IEN_ACK | I2C_IEN_NACK | I2C_IEN_SSTOP;
		if (i2c == I2C0)
			NVIC_EnableIRQ(I2C0_IRQn);
		else
			NVIC_EnableIRQ(I2C1_IRQn);
	}
	else
	{
		// Disable + clear interrupts and flags
		if (i2c == I2C0)
			NVIC_DisableIRQ(I2C0_IRQn);
		else
			NVIC_DisableIRQ(I2C1_IRQn);
		i2c->IEN = 0;
		i2c->IFC = ~0;
	}
}

/***************************************************************************//**
 * @author Pieter J. Botma
 * @date   23/05/2012
 *
 * This function starts a communication sequence, of type \b flag, with a slave
 * module, \b address, by transmitting \b txBuffer and or receiving \b rxBuffer
 * over the specified I2C channel.
 *
 * @param[in] i2c
 *   Pointer to the I2C module to be used.
 * @param[in] address
 *   I2C addres of slave to initiate communication with.
 * @param[in] flag
 *   Indicates the type of communication sequence.
 * @param[in] txBuffer
 *   Pointer to data buffer to be transmitted over the specified I2C channel.
 * @param[in] txBufferSize
 *   Size of data buffer to be tranmistted.
 * @param[out] rxBuffer
 *   Pointer to buffer where data, received over the specified I2C channel, should be saved.
 * @param[in] rxBufferSize
 *   Number of bytes received over the specified I2C channel.
 ******************************************************************************/
uint32_t BSP_I2C_masterTX (I2C_TypeDef *i2c, uint16_t address, BSP_I2C_ModeSelect_TydeDef flag,
		uint8_t *txBuffer, uint16_t txBufferSize,
		uint8_t *rxBuffer, uint16_t rxBufferSize)
{
	//i2cDisableSlaveInterrupts();
	//BSP_I2C_setSlaveMode(i2c,0);

	I2C_TransferReturn_TypeDef result;
	I2C_TransferSeq_TypeDef i2cTransfer;

	// Initializing I2C transfer
	i2cTransfer.addr        = address;
	i2cTransfer.flags       = flag;
	i2cTransfer.buf[0].data = txBuffer;
	i2cTransfer.buf[0].len  = txBufferSize;
	i2cTransfer.buf[1].data = rxBuffer;
	i2cTransfer.buf[1].len  = rxBufferSize;

	I2C_TransferInit(i2c, &i2cTransfer);

	uint32_t timeout = 0x0FFFFF;

	/* Sending data */
	do
	{
		timeout--;

		result = I2C_Transfer(i2c);
	}
	while (result == i2cTransferInProgress && timeout != 0);

	return timeout;
}

/***************************************************************************//**
 * @author Pieter J. Botma
 * @date   23/04/2014
 *
 * This function starts a communication sequence, of type \b flag, with a slave
 * module, \b address, by transmitting \b txBuffer and or receiving \b rxBuffer
 * over the specified I2C channel.
 *
 * @note
 *   The functions of the data buffers (buffer1 and buffer2) is defined by the
 *   communication sequence flag. The following options exist:
 *   - bspI2cWrite: buffer 1 is a transmit buffer. buffer 2 is not used.
 *   - bspI2cWriteWrite: buffer 1 is a transmit buffer. buffer 2 is a transmit buffer
 *   - bspI2cWriteRead: buffer 1 is a transmit buffer. buffer 2 is a receive buffer
 *   - bspI2CRead: buffer 1 is a receive buffer. buffer 2 is not used.
 *
 * @param[in] i2c
 *   Pointer to the I2C module to be used.
 * @param[in] address
 *   I2C addres of slave to initiate communication with.
 * @param[in] flag
 *   Indicates the type of communication sequence.
 * @param[in] buffer1
 *   Pointer to first data buffer used by the I2C transmission.
 * @param[in] buffer1Size
 *   Size of the first data buffer.
 * @param[out] buffer2
 *   Pointer to seconds data buffer used by the I2C transmission.
 * @param[in] buffer2Size
 *   Size of the seconds data buffer.
 *
 * @return
 *   The result of the I2C message transmission. If result equals 1
 *   (i2cTransferInProgress), a clock low timeout occurred, indicating that the
 *   slave is most likely keeping the clock line low.
 ******************************************************************************/
I2C_TransferReturn_TypeDef BSP_I2C_masterTransmit
(

		I2C_TypeDef *i2c, uint16_t address, BSP_I2C_ModeSelect_TydeDef flag,
		uint8_t *buffer1, uint16_t buffer1Size,
		uint8_t *buffer2, uint16_t buffer2Size
)
{
	I2C_TransferReturn_TypeDef result;
	I2C_TransferSeq_TypeDef i2cTransfer;

	// Initializing I2C transfer
	i2cTransfer.addr        = address;
	i2cTransfer.flags       = flag;
	i2cTransfer.buf[0].data = buffer1;
	i2cTransfer.buf[0].len  = buffer1Size;
	i2cTransfer.buf[1].data = buffer2;
	i2cTransfer.buf[1].len  = buffer2Size;

	I2C_TransferInit(i2c, &i2cTransfer);

	uint32_t timeout = 0;

	I2C_IntClear(i2c,I2C_IFC_CLTO);

	do
	{
		result = I2C_Transfer(i2c);

		timeout = I2C_IntGet(i2c) & I2C_IF_CLTO;
	}
	while (result == i2cTransferInProgress && !timeout);

	return result;
}

/** @} (end addtogroup I2C) */
/** @} (end addtogroup BSP_Library) */
