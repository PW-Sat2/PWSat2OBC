/***************************************************************************/ /**
  * @file	bsp_uart.c
  * @brief	BSP UART source file.
  *
  * This file contains all the implementations for the functions defined in \em
  * bsp_uart.h.
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

#include "bsp_uart.h"
#include <cstring>

/***************************************************************************/ /**
  * @addtogroup BSP_Library
  * @brief Board Support Package (<b>BSP</b>) Driver Library for CubeComputer.
  * @{
  ******************************************************************************/

/***************************************************************************/ /**
  * @addtogroup UART
  * @brief API for CubeComputer UART channels.
  * @{
  ******************************************************************************/

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

volatile uint8_t debugTxInProgress; // debug uart transmission flag

/***************************************************************************/ /**
  * @fn void debugTxComplete(unsigned int channel, bool primary, void *user)
  *
  * Callback function for Debug DMA TX channel. ( \em forward declaration )
  ******************************************************************************/
void debugTxComplete(unsigned int channel, bool primary, void* user)
{
    // Clear transmission in progress flag
    debugTxInProgress = 0;
}

void InitDebug(void)
{
    CMU_ClockEnable(BSP_UART_DEBUG_CLOCK, true);
    CMU_ClockEnable(cmuClock_GPIO, true);

    // Setting DMA call-back function

    cb[DMA_CHANNEL_DEBUG_TX].cbFunc = debugTxComplete;
    cb[DMA_CHANNEL_DEBUG_TX].userPtr = NULL;

    // DMA config

    DMA_CfgChannel_TypeDef chnlTxCfg;
    chnlTxCfg.highPri = false;                  // Normal priority
    chnlTxCfg.enableInt = true;                 // Interupt enabled for callback functions
    chnlTxCfg.select = BSP_UART_DEBUG_DMAREQ;   // Set USART0 TX empty avalible as source of DMA signals
    chnlTxCfg.cb = &(cb[DMA_CHANNEL_DEBUG_TX]); // Callback funtion // NULL
    DMA_CfgChannel(DMA_CHANNEL_DEBUG_TX, &chnlTxCfg);

    DMA_CfgDescr_TypeDef descrTxCfg;
    descrTxCfg.dstInc = dmaDataIncNone;
    descrTxCfg.srcInc = dmaDataInc1;
    descrTxCfg.size = dmaDataSize1;
    descrTxCfg.arbRate = dmaArbitrate1;
    descrTxCfg.hprot = 0;
    DMA_CfgDescr(DMA_CHANNEL_DEBUG_TX, true, &descrTxCfg);

    // UART config

    USART_InitAsync_TypeDef init = USART_INITASYNC_DEFAULT;
    init.baudrate = 112500;
    USART_Reset(BSP_UART_DEBUG);
    USART_InitAsync(BSP_UART_DEBUG, &init);

    // UART location and pin settings

    BSP_UART_DEBUG->ROUTE = USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | BSP_UART_DEBUG_LOC;

#if defined(CubeCompV2B)
    GPIO_PinModeSet(gpioPortE, 7, gpioModePushPull, 1);  // TX is on E7
    GPIO_PinModeSet(gpioPortE, 6, gpioModeInputPull, 1); // RX is on E6

#else
    GPIO_PinModeSet(gpioPortB, 9, gpioModePushPull, 1);   // TX is on B9
    GPIO_PinModeSet(gpioPortB, 10, gpioModeInputPull, 1); // RX is on B10
#endif

    // Setup interrupts
    BSP_UART_DEBUG->IEN = USART_IEN_RXDATAV;
    NVIC_EnableIRQ(BSP_UART_DEBUG_RX_IRQn);

    // Initialise transfer flag
    debugTxInProgress = 0;
}

void InitMisc(void)
{
    CMU_ClockEnable(BSP_UART_MISC_CLOCK, true);
    CMU_ClockEnable(cmuClock_GPIO, true);

    // Setting DMA call-back function

    cb[DMA_CHANNEL_DEBUG_TX].cbFunc = debugTxComplete;
    cb[DMA_CHANNEL_DEBUG_TX].userPtr = NULL;

    // DMA config

    DMA_CfgChannel_TypeDef chnlTxCfg;
    chnlTxCfg.highPri = false;
    chnlTxCfg.enableInt = true;
    chnlTxCfg.select = BSP_UART_MISC_DMAREQ;
    chnlTxCfg.cb = &(cb[DMA_CHANNEL_DEBUG_TX]);
    ;
    DMA_CfgChannel(DMA_CHANNEL_DEBUG_TX, &chnlTxCfg);

    DMA_CfgDescr_TypeDef descrTxCfg;
    descrTxCfg.dstInc = dmaDataIncNone;
    descrTxCfg.srcInc = dmaDataInc1;
    descrTxCfg.size = dmaDataSize1;
    descrTxCfg.arbRate = dmaArbitrate1;
    descrTxCfg.hprot = 0;
    DMA_CfgDescr(DMA_CHANNEL_DEBUG_TX, true, &descrTxCfg);

    // UART config

    USART_InitAsync_TypeDef init = USART_INITASYNC_DEFAULT;
    init.baudrate = 115200;
    USART_Reset(BSP_UART_MISC);
    USART_InitAsync(BSP_UART_MISC, &init);

    // UART location and pin settings

    BSP_UART_MISC->ROUTE = USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | BSP_UART_MISC_LOC;

#if defined(CubeCompV2B)
    GPIO_PinModeSet(gpioPortB, 3, gpioModePushPull, 1);  // TX is on B3
    GPIO_PinModeSet(gpioPortB, 4, gpioModeInputPull, 1); // RX is on B4

#else
    GPIO_PinModeSet(gpioPortF, 6, gpioModePushPull, 1);  // TX is on F6
    GPIO_PinModeSet(gpioPortF, 7, gpioModeInputPull, 1); // RX is on F7
#endif

    // Setup interrupts
    BSP_UART_MISC->IEN = USART_IEN_RXDATAV;
    NVIC_EnableIRQ(BSP_UART_MISC_RX_IRQn);

    // Initialise transfer flag
    debugTxInProgress = 0;
}

/** @endcond */

/***************************************************************************/ /**
  * @author Pieter J. Botma
  * @date   28/03/2012
  *
  * This function initialises the specified UART channel with the following
  * properties: 115200-8-N-1
  * @param[in] usart
  *   Pointer to UART to be used.
  ******************************************************************************/
void BSP_UART_Init(USART_TypeDef* usart)
{
    // Initialise the specified UART
    if (usart == BSP_UART_DEBUG)
        InitDebug();
    else if (usart == BSP_UART_MISC)
        InitMisc();
}

/***************************************************************************/ /**
  * @author Pieter J. Botma
  * @date   10/05/2012
  *
  * This function transmits the data byte, \b data, over the
  * specified UART channel.
  * @param[in] usart
  *	 Pointer to UART to be used.
  * @param[in] data
  * 	 Byte of data to be transmitted.
  ******************************************************************************/
void BSP_UART_txByte(USART_TypeDef* usart, uint8_t data)
{
    USART_Tx(usart, data);
}

/***************************************************************************/ /**
  * @author Pieter J. Botma
  * @date   10/05/2012
  *
  * This function transmits the data buffer, \b buff of size \b len, over the
  * specified UART channel using DMA.
  * @param[in] usart
  *   Pointer to UART to be used.
  * @param[in] buff
  *	 Pointer to data buffer to be transmitted.
  * @param[in] len
  * 	 Length of data buffer to be transmitted.
  * @param[in] wait
  *   Determines if function should wait for transmission to finish.
  ******************************************************************************/
void BSP_UART_txBuffer(USART_TypeDef* usart, const uint8_t* buff, uint16_t len, bool wait)
{
    // Implement transmission using DMA to free up MCU
    DMA_ActivateBasic(DMA_CHANNEL_DEBUG_TX, // activate channel selected
        true,                               // use primary descriptor
        false,                              // no DMA burst
        (void*)&(usart->TXDATA),            // destination address
        (void*)(buff),                      // source address
        len - 1);                           // buffer length -1

    // Set transmission flag as busy
    debugTxInProgress = 1;

    // Wait for previous transfer to complete
    if (wait)
        while (debugTxInProgress)
            ;
}

void BSP_UART_Puts(USART_TypeDef* usart, const char* str)
{
    auto length = strlen(str);
    BSP_UART_txBuffer(usart, reinterpret_cast<const uint8_t*>(str), length, true);
}

/***************************************************************************/ /**
  * @author Pieter J. Botma
  * @date   01/08/2013
  *
  * This function returns the UART DMA transmission status flag.
  * @return
  *   Returns the UART DMA transmission status flag.
  ******************************************************************************/
bool BSP_UART_txInProgress(void)
{
    return debugTxInProgress;
}

/***************************************************************************/ /**
  * @author Pieter J. Botma
  * @date   28/03/2012
  *
  * This function returns a data byte over the
  * specified UART channel.
  * @param[in] usart
  *   Pointer to UART to be used.
  *
  ******************************************************************************/
uint8_t BSP_UART_rxByte(USART_TypeDef* usart)
{
    // Disable interrupts
    usart->IEN &= ~USART_IF_RXDATAV;

    uint8_t data = USART_Rx(usart);

    // Enable interrupts
    USART_IntClear(usart, USART_IF_RXDATAV);
    usart->IEN |= USART_IF_RXDATAV;

    return data;
}

/***************************************************************************/ /**
  * @author Pieter J. Botma
  * @date   28/03/2012
  *
  * This function receives a data buffer, \b buff of size \b len, over the
  * specified UART channel.
  *
  * @param[in] usart
  *   Pointer to UART to be used.
  * @param[out] buff
  *	 Pointer to data buffer where received data should be placed.
  * @param[in] len
  * 	 Length of data buffer to be received.
  ******************************************************************************/
void BSP_UART_rxBuffer(USART_TypeDef* usart, uint8_t* buff, uint8_t len)
{
    uint32_t index, flags;

    // Disable interrupts
    flags = USART_IntGetEnabled(usart);
    USART_IntClear(usart, flags);

    // Disable interrupts
    // USART_IntDisable(usart,USART_IF_RXDATAV);

    // Receiving data
    for (index = 0; index < len; index++)
    {
        buff[index] = USART_Rx(usart);
    }

    // Enable interrupts
    USART_IntEnable(usart, flags);
}

/** @} (end addtogroup UART) */
/** @} (end addtogroup BSP_Library) */
