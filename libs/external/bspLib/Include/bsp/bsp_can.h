/***************************************************************************//**
 * @file	bsp_can.h
 * @brief	BSP CAN header file.
 *
 * This header file contains all the required definitions and function
 * prototypes through which to control CubeComputer CAN interface.
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
#ifndef BSP_CAN_H_
#define BSP_CAN_H_
#include <stdint.h>
#include "MCP2515.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_usart.h"

/***************************************************************************//**
 * @addtogroup BSP_Library
 * @brief Board Support Package (<b>BSP</b>) Driver Library for CubeComputer.
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup CAN
 * @brief API for CubeComputer CAN interface.
 * @{
 ******************************************************************************/

#define BSP_CAN_USART           USART1											///< USART channel used for MCP2515 SPI interface
#define BSP_CAN_CMUCLOCK        cmuClock_USART1									///< USART clock used for MCP2515 SPI interface
#define BSP_CAN_LOC             USART_ROUTE_LOCATION_LOC1						///< USART pinnout location used for MCP2515 SPI interface

#define BSP_CAN_GPIOPORT_SPI   	gpioPortD										///< GPIO port used for MCP2515 SPI interface
#define BSP_CAN_MOSIPIN         0												///< GPIO port pin used for MCP2515 SPI MOSI
#define BSP_CAN_MISOPIN         1												///< GPIO port pin used for MCP2515 SPI MISO
#define BSP_CAN_CLKPIN          2												///< GPIO port pin used for MCP2515 SPI CLK
#define BSP_CAN_CSPIN           3												///< GPIO port pin used for MCP2515 SPI CS

#define BSP_CAN_GPIOPORT_MSC	gpioPortC										///< GPIO port used for MCP2515 miscellaneous interface
#define BSP_CAN_nRSTPIN			11												///< GPIO port pin used for MCP2515 reset pin
#define BSP_CAN_OSCPIN			12												///< GPIO port pin used for MCP2515 clock pin
#define BSP_CAN_nINTPIN			13												///< GPIO port pin used for MCP2515 interrupt pin

#define BSP_CAN_OSCSEL			CMU_CTRL_CLKOUTSEL0_HFCLK2						///< MCU clock out frequency select for MCP2515 clock input
#define BSP_CAN_OSCOUT			CMU_ROUTE_CLKOUT0PEN							///< MCU clock out enable select for MCP2515 clock input
#define BSP_CAN_OSCLOC			CMU_ROUTE_LOCATION_LOC1							///< MCU clock out location select for MCP2515 clock input

#define BSP_CAN_1MBPS_CNF1		( SJW1 | (1-1) )								///< MCP2515 Bit timing register 1
#define BSP_CAN_1MBPS_CNF2		( BTLMODE | SAMPLE_1X | ((5-1)<<3) | (2-1) )	///< MCP2515 Bit timing register 2
#define BSP_CAN_1MBPS_CNF3		( SOF_DISABLE | WAKFIL_DISABLE | (4-1) )		///< MCP2515 Bit timing register 3

#define BSP_CAN_RX0IDMASK		0x700											///< CubeComputer CAN standard ID mask
#define BSP_CAN_RX0IDFILTER		0x500											///< CubeComputer CAN standard ID filter

#define BSP_CAN_RX0MASK_SIDH	0xE0											///< MCP2515 RX buffer 0 standard ID mask register 1
#define BSP_CAN_RX0MASK_SIDL	0x00											///< MCP2515 RX buffer 1 standard ID mask register 2

#define BSP_CAN_id2RegH(id) 	( (uint8_t) ( (id >> 3) & 0x0FF ) )				///< Convert standard ID to MCP2515 SIDH format
#define BSP_CAN_id2RegL(id) 	( (uint8_t) ( (id << 5) & 0x0E0 ) )				///< Convert standard ID to MCP2515 SIDL format
#define BSP_CAN_reg2Id(idH,idL) ( ((idH << 3) & 0x7F8) | ((idL >> 5) & 0x007) )	///< Retrieve standard ID from MCP2515 register format

#define BSP_CAN_select()		GPIO_PinOutClear(BSP_CAN_GPIOPORT_SPI, BSP_CAN_CSPIN) ///< MCP2515 CS pin low/enable
#define BSP_CAN_deselect()		GPIO_PinOutSet(BSP_CAN_GPIOPORT_SPI, BSP_CAN_CSPIN)   ///< MCP2515 CS pin high/disable

#define BSP_CAN_DUMMY_TX 		0xFF											///< SPI dummy transmit data in order to receive data

void 	BSP_CAN_reset (void);													///< MCP2515 SPI reset command
void 	BSP_CAN_write (uint8_t addr, uint8_t data);								///< MCP2515 SPI write command
void 	BSP_CAN_writeBuffer (uint8_t addr, uint8_t *buffer, uint8_t len);		///< MCP2515 SPI write buffer command
void 	BSP_CAN_writeTxBuffer (BSP_CAN_TxLoadAddr_TypeDef startAddr, uint8_t *buffer); ///< MCP2515 SPI write TX buffer command
void 	BSP_CAN_requestToSend (BSP_CAN_RequestToSend_TypeDef rts);				///< MCP2515 SPI request to send command
uint8_t BSP_CAN_read (uint8_t addr);											///< MCP2515 SPI read command
void 	BSP_CAN_readBuffer (uint8_t addr, uint8_t *buffer, uint8_t len);		///< MCP2515 SPI read buffer command
void 	BSP_CAN_readRxBuffer (BSP_CAN_RxReadAddr_TypeDef startAddr, uint8_t *buffer); ///< MCP2515 SPI read RX buffer
uint8_t BSP_CAN_readStatus (void);												///< MCP2515 SPI read status command
uint8_t BSP_CAN_readRxStatus (void);											///< MCP2515 SPI read RX status command
void 	BSP_CAN_Init(void);														///< Initialize MCU SPI interface and MCP2515 registers

/** @} (end addtogroup UART) */
/** @} (end addtogroup BSP_Library) */

#endif /* BSP_CAN_H_ */
