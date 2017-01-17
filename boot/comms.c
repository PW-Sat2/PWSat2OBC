/*
 * comms.c
 *
 *  Created on: 30 Jul 2013
 *      Author: pjbotma
 */

#include "comms.h"

#define UPLOADBLOCKSIZE 256

uint8_t msgId;

uint8_t debugStr[256], debugLen;

uint8_t uartReceived;

uint8_t uploadIndex;
uint16_t uploadBlockCRC;


void COMMS_Init(void)
{
	BSP_UART_Init(BSP_UART_DEBUG);

	msgId = 0x00;

	uartReceived = 0;
}


void COMMS_processMsg(void)
{
	uint8_t  index, entry, *data;
	uint32_t len, i;
	uint8_t  *crcStart, *crcEnd;
	uint16_t crcVal;
	uint8_t desc[BOOT_ENTRY_DESCRIPTION_SIZE];

	// Disable UART interrupts
	BSP_UART_DEBUG->IEN &= ~USART_IF_RXDATAV;

	switch(msgId)
	{
	case 'b': //

		uartReceived = 0;

		// reset message id
		msgId = 0x00;

		break;


	case 's': // set boot index

		uartReceived = 1;

		debugLen = sprintf((char*)debugStr,"\n\nNew Boot Index: ");
		BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)debugStr,debugLen, true);

		// get boot index
		index = USART_Rx(BSP_UART_DEBUG);

		debugLen = sprintf((char*)debugStr,"%d",index);
		BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)debugStr,debugLen, true);

		// test boot index
		if (index > BOOT_TABLE_SIZE)
		{
			debugLen = sprintf((char*)debugStr,"\n\nError: Boot index out of bounds!");
			BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)debugStr,debugLen, true);
			return;
		}

	    // upload new program to internal flash
	    BOOT_setBootIndex(index);
	    BOOT_resetBootCounter();

	    debugLen = sprintf((char*)debugStr,"...Done!");
	    BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)debugStr,debugLen, true);

	    // reset message id
	    msgId = 0x00;

		break;

	case 'l': // list entries

		uartReceived = 1;

		debugLen = sprintf((char*)debugStr,"\n\nBoot Table Entries:\n");
		BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)debugStr,debugLen, true);

		for (entry = 1; entry <= BOOT_TABLE_SIZE; entry++)
		{
			debugLen = sprintf((char*)debugStr,"\n%d. ", entry);
			BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)debugStr,debugLen, true);

			// check if entry is valid
			if( *(uint8_t*)( BOOT_TABLE_BASE + BOOT_getOffsetValid(entry) ) != BOOT_ENTRY_ISVALID )
			{
				debugLen = sprintf((char*)debugStr,"Not Valid!");
				BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)debugStr,debugLen, true);

				continue;
			}

			// display description
			for(i = 0; i < BOOT_ENTRY_DESCRIPTION_SIZE; i++)
			{
				data = (uint8_t*)( BOOT_TABLE_BASE + BOOT_getOffsetDescription(entry) + i );

				if(*data == '\r') { break; }

				USART_Tx(BSP_UART_DEBUG, *data);
			}
		}

		// reset message id
		msgId = 0x00;

		break;

	case 'x': // upload to boot table

		uartReceived = 1;

		debugLen = sprintf((char*)debugStr,"\n\nBoot Index: ");
		BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)debugStr,debugLen, true);

		// get boot index
		index = USART_Rx(BSP_UART_DEBUG);

		debugLen = sprintf((char*)debugStr,"%d",index);
		BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)debugStr,debugLen, true);

		// test boot index
		if(index == 0x00)
		{
			debugLen = sprintf((char*)debugStr,"\nError: Cant override safe mode program!");
			BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)debugStr,debugLen, true);
			return;
		}
		if (index > BOOT_TABLE_SIZE)
		{
			debugLen = sprintf((char*)debugStr,"\nError: Boot index out of bounds!");
			BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)debugStr,debugLen, true);
			return;
		}

		debugLen = sprintf((char*)debugStr,"\nUpload Binary: ");
		BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)debugStr,debugLen, true);

		len = XMODEM_upload(index);

		// update entry length
		BOOT_setLen(index, len);

	    // calculate actual CRC of downloaded application
	    crcStart = (uint8_t *)( BOOT_TABLE_BASE + BOOT_getOffsetProgram(index) );
	    crcEnd   = crcStart + len;
	    crcVal   = CRC_calc(crcStart, crcEnd);

	    // update entry crc
	    BOOT_setCRC(index, crcVal);

	    // mark entry as valid
	    BOOT_setValid(index);

	    debugLen = sprintf((char*)debugStr,"\nBoot Description: ");
	    BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)debugStr,debugLen, true);

	    // get description
	    for(i = 0; i < BOOT_ENTRY_DESCRIPTION_SIZE; i++)
	    {
	    	desc[i] = USART_Rx(BSP_UART_DEBUG);

	    	USART_Tx(BSP_UART_DEBUG, desc[i]);

	    	if(desc[i] == '\n') { break; }
	    }

	    // update entry description
	    BOOT_programDescription (index, desc);

	    // change boot index
	    BOOT_setBootIndex(index);
	    BOOT_resetBootCounter();

	    debugLen = sprintf((char*)debugStr,"...Done!");
	    BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)debugStr,debugLen, true);

	    // reset message id
	    msgId = 0x00;

		break;

	case 'r': // Reset MCU

		SCB->AIRCR = 0x05FA0004;

		break;

	case 'z': // upload safe mode

		uartReceived = 1;

		// get boot index
		index = 0;

		// upload safe mode application
		len = XMODEM_upload(index);

		// change boot index
		BOOT_setBootIndex(index);
		BOOT_resetBootCounter();

		// reset message id
		msgId = 0x00;

		break;

	default:
		// reset message id
		msgId = 0x00;
		break;

	}

    // Enable UART interrupts
    USART_IntClear(BSP_UART_DEBUG, USART_IF_RXDATAV);
    BSP_UART_DEBUG->IEN |= USART_IF_RXDATAV;
}


/**
 * UART interrupt handler
 */
void BSP_UART_DEBUG_IRQHandler(void)
{
	uint8_t temp;

	// disable interrupt
	BSP_UART_DEBUG->IEN &= ~USART_IEN_RXDATAV;

	// only save message if its been processed (i.e. 0x00)
	if (msgId == 0x00)
	{
		// save message id
		msgId = BSP_UART_DEBUG->RXDATA;
	}
	else
	{
		// store in dummy variable to clear flag
		temp = BSP_UART_DEBUG->RXDATA;
	}

	// enable interrupt
	BSP_UART_DEBUG->IEN |= USART_IEN_RXDATAV;
}
