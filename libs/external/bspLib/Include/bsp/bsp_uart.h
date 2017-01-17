/***************************************************************************//**
 * @file	bsp_uart.h
 * @brief	BSP UART header file.
 *
 * This header file contains all the required definitions and function
 * prototypes through which to control CubeComputer UART channels.
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

#ifndef __BSP_UART_H
#define __BSP_UART_H

#include "em_usart.h"
#include "bsp_dma.h"
#include "em_cmu.h"
#include "em_gpio.h"

/***************************************************************************//**
 * @addtogroup BSP_Library
 * @brief Board Support Package (<b>BSP</b>) Driver Library for CubeComputer.
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup UART
 * @brief API for CubeComputer UART channels.
 * @{
 ******************************************************************************/

#if defined(CubeCompV2B)

#define	BSP_UART_DEBUG 			USART0  					///< Debug UART channel mapped to MCU USART0.
#define BSP_UART_DEBUG_CLOCK 	cmuClock_USART0				///< Debug UART clock select
#define BSP_UART_DEBUG_LOC		USART_ROUTE_LOCATION_LOC1	///< Debug UART route location
#define BSP_UART_DEBUG_DMAREQ	DMAREQ_USART0_TXEMPTY		///< Debug UART DMA request trigger on TX buffer empty.
#define BSP_UART_DEBUG_RX_IRQn	USART0_RX_IRQn				///< Debug UART IRQ number in vector table.

#define	BSP_UART_MISC  			USART2  					///< Miscellaneous UART channel mapped to MCU USART1.
#define BSP_UART_MISC_CLOCK		cmuClock_USART2  			///< Miscellaneous UART clock select
#define BSP_UART_MISC_LOC		USART_ROUTE_LOCATION_LOC1	///< Miscellaneous UART route location
#define BSP_UART_MISC_DMAREQ	DMAREQ_USART2_TXEMPTY		///< Miscellaneous UART DMA request trigger on TX buffer empty.
#define BSP_UART_MISC_RX_IRQn	USART2_RX_IRQn				///< Miscellaneous UART IRQ number in vector table

#define BSP_UART_DEBUG_IRQHandler(void) USART0_RX_IRQHandler(void) ///< Redefine IRQ handler function name

#else

#define	BSP_UART_DEBUG 			UART1  						///< Debug UART channel mapped to MCU USART0.
#define BSP_UART_DEBUG_CLOCK 	cmuClock_UART1				///< Debug UART clock select
#define BSP_UART_DEBUG_LOC		UART_ROUTE_LOCATION_LOC2	///< Debug UART route location
#define BSP_UART_DEBUG_DMAREQ	DMAREQ_UART1_TXEMPTY		///< Debug UART DMA request trigger on TX buffer empty.
#define BSP_UART_DEBUG_RX_IRQn	UART1_RX_IRQn				///< Debug UART IRQ number in vector table.

#define	BSP_UART_MISC  			UART0  						///< Miscellaneous UART channel mapped to MCU USART1.
#define BSP_UART_MISC_CLOCK		cmuClock_UART0  			///< Miscellaneous UART clock select
#define BSP_UART_MISC_LOC		UART_ROUTE_LOCATION_LOC0	///< Miscellaneous UART route location
#define BSP_UART_MISC_DMAREQ	DMAREQ_UART0_TXEMPTY		///< Miscellaneous UART DMA request trigger on TX buffer empty.
#define BSP_UART_MISC_RX_IRQn	UART0_RX_IRQn				///< Miscellaneous UART IRQ number in vector table

#define BSP_UART_DEBUG_IRQHandler(void) UART1_RX_IRQHandler(void) ///< Redefine IRQ handler function name

#endif


void    BSP_UART_Init     (USART_TypeDef *usart); 							  				///< Initialise specified UART.
void    BSP_UART_txByte   (USART_TypeDef *usart, uint8_t data); 			  				///< Transmit one byte of data over specified UART.
void    BSP_UART_txBuffer (USART_TypeDef *usart, uint8_t *buff, uint16_t len, bool wait); 	///< Transmit data buffer over specified UART.
bool 	BSP_UART_txInProgress (void);														///< Returns the progress of the UART DMA transmission
uint8_t BSP_UART_rxByte   (USART_TypeDef *usart); 							  				///< Receive a byte of data over specified UART.
void    BSP_UART_rxBuffer (USART_TypeDef *usart, uint8_t *buff, uint8_t len); 				///< Receive data buffer over specified UART.

/** @} (end addtogroup UART) */
/** @} (end addtogroup BSP_Library) */

#endif // __BSP_UART_H

