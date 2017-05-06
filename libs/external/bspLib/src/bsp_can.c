/***************************************************************************//**
 * @file	bsp_can.c
 * @brief	BSP CAN source file.
 *
 * This file contains all the implementations for the functions defined in \em
 * bsp_can.h.
 * @author	Pieter J. Botma
 * @date	13 Aug 2013
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

/***************************************************************************//**
 * @addtogroup BSP_Library
 * @brief Board Support Package (<b>BSP</b>) Driver Library for CubeComputer.
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup CAN
 * @brief API for CubeComputer CAN channels.
 * @{
 ******************************************************************************/

#include "bsp_can.h"

/***************************************************************************//**
 * @author Pieter J. Botma
 * @date   19/08/2013
 *
 * This function generates & send a SPI reset command to the MCP2515 CAN
 * controller.
 ******************************************************************************/
void BSP_CAN_reset (void)
{
	BSP_CAN_select();

	USART_SpiTransfer(BSP_CAN_USART, MCP_RESET );

	BSP_CAN_deselect();
}

/***************************************************************************//**
 * @author Pieter J. Botma
 * @date   19/08/2013
 *
 * This function generates & send a SPI write command to the MCP2515 CAN
 * controller. The specified value (data) is written to the specified address
 * (addr).
 * @param[in] addr
 *   MCP2515 register addres to be written to.
 * @param[in] data
 *   Value of data to be written to MCP2515 register.
 ******************************************************************************/
void BSP_CAN_write (uint8_t addr, uint8_t data)
{
	BSP_CAN_select();

	USART_SpiTransfer(BSP_CAN_USART, MCP_WRITE );
	USART_SpiTransfer(BSP_CAN_USART, addr );
	USART_SpiTransfer(BSP_CAN_USART, data );

	BSP_CAN_deselect();
}

/***************************************************************************//**
 * @author Pieter J. Botma
 * @date   19/08/2013
 *
 * This function generates & send a SPI write command to the MCP2515 CAN
 * controller. The specified values (buffer, len) are written sequentially to
 * the registers starting at the specified address (addr).
 * @param[in] addr
 *   MCP2515 register address the sequential writes should be started at.
 * @param[in] buffer
 *   Buffer containing the values to be written to the MCP2515 registers.
 * @param[in] len
 *   Length of the buffer of values to be written to the MCP2515 registers.
 ******************************************************************************/
void BSP_CAN_writeBuffer (uint8_t addr, uint8_t *buffer, uint8_t len)
{
	uint8_t i = 0;

	BSP_CAN_select();

	USART_SpiTransfer(BSP_CAN_USART, MCP_WRITE );
	USART_SpiTransfer(BSP_CAN_USART, addr );

	for (i = 0; i < len; i++)
		USART_SpiTransfer(BSP_CAN_USART, buffer[i]);

	BSP_CAN_deselect();
}

/***************************************************************************//**
 * @author Pieter J. Botma
 * @date   19/08/2013
 *
 * This function generate & sends a SPI write command to the MCP2515 CAN
 * controller. The specified values (buffer) is written sequentially to the
 * MCP2515, starting at the specified TX buffer register address (startAddr).
 * @param[in] startAddr
 *   MCP2515 TX buffer the sequential writes should be started at.
 * @param[in] buffer
 *   Buffer containing the values to be written to the MCP2515 TX buffer.
 ******************************************************************************/
void BSP_CAN_writeTxBuffer (BSP_CAN_TxLoadAddr_TypeDef startAddr, uint8_t *buffer)
{
	uint8_t i, len;

	switch(startAddr)
	{
	case bspCanTx0Id:
	case bspCanTx1Id:
	case bspCanTx2Id:
		len = 13;
		break;

	case bspCanTx0Data:
	case bspCanTx1Data:
	case bspCanTx2Data:
		len = 8;
		break;

	default:
		len = 0;
		break;
	}

	BSP_CAN_select();

	USART_SpiTransfer(BSP_CAN_USART, (uint8_t) startAddr );

	for (i = 0; i < len; i++)
		USART_SpiTransfer(BSP_CAN_USART, buffer[i]);

	BSP_CAN_deselect();
}

/***************************************************************************//**
 * @author Pieter J. Botma
 * @date   19/08/2013
 *
 * This function generates and send a SPI request-to-send command to the MCP2515
 * CAN controller for a specified TX buffer (rtsSelect).
 * @param[in] rtsSelect
 *   MCP2515 TX buffer the sequential writes should be started at.
 ******************************************************************************/
void BSP_CAN_requestToSend (BSP_CAN_RequestToSend_TypeDef rtsSelect)
{
	BSP_CAN_select();

	USART_SpiTransfer(BSP_CAN_USART, (uint8_t) rtsSelect );

	BSP_CAN_deselect();
}


/***************************************************************************//**
 * @author Pieter J. Botma
 * @date   19/08/2013
 *
 * This function generates and send a SPI read command to the MCP2515 CAN
 * controller. The value returned is read from the register at the specified
 * register addres (addr).
 * @param[in] addr
 *   The address of the register value to be read.
 * @return
 *   The value at the specified register address.
 ******************************************************************************/
uint8_t BSP_CAN_read (uint8_t addr)
{
	uint8_t data;

	BSP_CAN_select();

	USART_SpiTransfer(BSP_CAN_USART, MCP_READ );
	USART_SpiTransfer(BSP_CAN_USART, addr );

	data = USART_SpiTransfer(BSP_CAN_USART, BSP_CAN_DUMMY_TX);

	BSP_CAN_deselect();

	return data;
}

/***************************************************************************//**
 * @author Pieter J. Botma
 * @date   19/08/2013
 *
 * This function generates and send a SPI read command to the MCP2515 CAN
 * controller. The register values are read sequentially starting at the
 * specified register address
 * @param[in] addr
 *   The address of the register value to be read.
 * @param[out] buffer
 *   Pointer to the buffer the register values should be placed in.
 * @param[in] len
 *   The number of register values that should be read.
 ******************************************************************************/
void BSP_CAN_readBuffer (uint8_t addr, uint8_t *buffer, uint8_t len)
{
	uint8_t i = 0;

	BSP_CAN_select();

	USART_SpiTransfer(BSP_CAN_USART, MCP_READ );
	USART_SpiTransfer(BSP_CAN_USART, addr );

	for (i = 0; i < len; i++)
		buffer[i] = USART_SpiTransfer(BSP_CAN_USART, BSP_CAN_DUMMY_TX);

	BSP_CAN_deselect();
}

/***************************************************************************//**
 * @author Pieter J. Botma
 * @date   19/08/2013
 *
 * This function generates and send a SPI read RX command to the MCP2515 CAN
 * controller. The register values in the specified RX buffer (startAddr) are
 * read sequentially and placed in the specified buffer (buffer).
 * @param[in] startAddr
 *   MCP2515 RX buffer the sequential reads should be started at.
 * @param[out] buffer
 *   Pointer to the buffer the register values should be placed in.
 ******************************************************************************/
void BSP_CAN_readRxBuffer (BSP_CAN_RxReadAddr_TypeDef startAddr, uint8_t *buffer)
{
	uint8_t i, len;

	switch(startAddr)
	{
	case bspCanRx0Id:
	case bspCanRx1Id:
		len = 13;
		break;

	case bspCanRx0Data:
	case bspCanRx1Data:
		len = 8;
		break;

	default:
		len = 0;
		break;
	}

	BSP_CAN_select();

	USART_SpiTransfer(BSP_CAN_USART, (uint8_t) startAddr );

	for (i = 0; i < len; i++)
		buffer[i] = USART_SpiTransfer(BSP_CAN_USART, BSP_CAN_DUMMY_TX);

	BSP_CAN_deselect();
}

/***************************************************************************//**
 * @author Pieter J. Botma
 * @date   19/08/2013
 *
 * This function generates and send a SPI read status command to the MCP2515 CAN
 * controller. The status instruction allows single instruction access to some
 * of the often used status bits for message reception and transmission.
 * @return
 *   A collection of the most often used status bits.
 ******************************************************************************/
uint8_t BSP_CAN_readStatus (void)
{
	uint8_t status;

	BSP_CAN_select();

	USART_SpiTransfer(BSP_CAN_USART, MCP_READ_STATUS );

	status = USART_SpiTransfer(BSP_CAN_USART, BSP_CAN_DUMMY_TX);

	BSP_CAN_deselect();

	return status;
}

/***************************************************************************//**
 * @author Pieter J. Botma
 * @date   19/08/2013
 *
 * This function generates and send a SPI read RX status command to the MCP2515
 * CAN controller. The RX status is used to quickly determine which filter
 * matched the message and message type.
 * @return
 *   The RX status
 ******************************************************************************/
uint8_t BSP_CAN_readRxStatus (void)
{
	uint8_t status;

	BSP_CAN_select();

	USART_SpiTransfer(BSP_CAN_USART, MCP_RX_STATUS );

	status = USART_SpiTransfer(BSP_CAN_USART, BSP_CAN_DUMMY_TX);

	BSP_CAN_deselect();

	return status;
}

/***************************************************************************//**
 * @author Pieter J. Botma
 * @date   19/08/2013
 *
 * This function initialises the CAN interface on CubeComputer. Firstly the SPI
 * and miscellaneous pins are setup in order to communicate with MCP2515 CAN
 * controller. Secondly the MCP2515 registers are initialized to let the CAN bus
 * operate at 1Mbps. The masks and filters for one RX and TX buffer are
 * initialised. The RX interrupt pin is also configured.
 ******************************************************************************/
void BSP_CAN_Init(void)
{
	uint16_t i;
	uint8_t status;

	/************
	 * INIT SPI *
	 ************/
	// Setup clocks
	CMU_ClockEnable(BSP_CAN_CMUCLOCK, true);
	CMU_ClockEnable(cmuClock_GPIO, true);

	// Setup SPI
	USART_InitSync_TypeDef init = USART_INITSYNC_DEFAULT;
	init.baudrate = 100000;
	init.clockMode = usartClockMode0;
	init.msbf = true;
	USART_Reset(BSP_CAN_USART);
	USART_InitSync(BSP_CAN_USART, &init);

	// UART location and pin settings
	BSP_CAN_USART->ROUTE = USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | USART_ROUTE_CLKPEN | BSP_CAN_LOC;

	// Setup GPIO
	GPIO_PinModeSet(BSP_CAN_GPIOPORT_SPI, BSP_CAN_MOSIPIN, gpioModePushPull,  0);  // MOSI
	GPIO_PinModeSet(BSP_CAN_GPIOPORT_SPI, BSP_CAN_MISOPIN, gpioModeInputPull, 1);  // MISO
	GPIO_PinModeSet(BSP_CAN_GPIOPORT_SPI, BSP_CAN_CSPIN,   gpioModePushPull,  1);  // nCS
	GPIO_PinModeSet(BSP_CAN_GPIOPORT_SPI, BSP_CAN_CLKPIN,  gpioModePushPull,  0);  // CLK

	/****************
	 * INIT MCP2515 *
	 ****************/
	GPIO_PinModeSet(BSP_CAN_GPIOPORT_MSC, BSP_CAN_nRSTPIN, gpioModePushPull,  1);  	// nRST
	GPIO_PinModeSet(BSP_CAN_GPIOPORT_MSC, BSP_CAN_OSCPIN,  gpioModePushPull,  0);  	// CLK
	GPIO_PinModeSet(BSP_CAN_GPIOPORT_MSC, BSP_CAN_nINTPIN, gpioModeInputPull, 1); 	// nINT

	// MCP Clock Setup
	CMU->CTRL  |= BSP_CAN_OSCSEL;
	CMU->ROUTE |= BSP_CAN_OSCOUT | BSP_CAN_OSCLOC;

	// MCP INT pin Setup
	NVIC_EnableIRQ(GPIO_ODD_IRQn);
	GPIO_IntConfig(BSP_CAN_GPIOPORT_MSC, BSP_CAN_nINTPIN, false, true, true);

	// MCP Reset
	GPIO_PinOutClear(BSP_CAN_GPIOPORT_MSC, BSP_CAN_nRSTPIN);

	//BSP_CAN_reset ();

	// Wait 1ms (3 loops = 1us, i.e. 3000 loops)
	for(i = 0; i < 3000; i++);

	// MCP Enable
	GPIO_PinOutSet(BSP_CAN_GPIOPORT_MSC, BSP_CAN_nRSTPIN);

	status = BSP_CAN_read(MCP_CANSTAT);

	// MCP bit timing setup
	BSP_CAN_write(MCP_CNF1, BSP_CAN_1MBPS_CNF1);
	BSP_CAN_write(MCP_CNF2, BSP_CAN_1MBPS_CNF2);
	BSP_CAN_write(MCP_CNF3, BSP_CAN_1MBPS_CNF3);

	// MCP RX masks and filter setup
	BSP_CAN_write( MCP_RXM0SIDH, BSP_CAN_id2RegH(BSP_CAN_RX0IDMASK) );
	BSP_CAN_write( MCP_RXM0SIDL, BSP_CAN_id2RegL(BSP_CAN_RX0IDMASK) );
	BSP_CAN_write( MCP_RXF0SIDH, BSP_CAN_id2RegH(BSP_CAN_RX0IDFILTER) );
	BSP_CAN_write( MCP_RXF0SIDL, BSP_CAN_id2RegL(BSP_CAN_RX0IDFILTER) );

	// set mask and filter for RX1 => 0x000
	BSP_CAN_write( MCP_RXM1SIDH, BSP_CAN_id2RegH(0x0FFF) );
	BSP_CAN_write( MCP_RXM1SIDL, BSP_CAN_id2RegL(0x0FFF) );

	// MCP RX interrupt and operation setup
	BSP_CAN_write( MCP_CANINTE, MCP_RX_INT ); // interrupts
	BSP_CAN_write( MCP_CANCTRL, ( MODE_NORMAL | CLKOUT_DISABLE ) ); // control register
}

/** @} (end addtogroup CAN) */
/** @} (end addtogroup BSP_Library) */
