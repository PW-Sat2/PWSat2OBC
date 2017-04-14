/*
 * comms.c
 *
 *  Created on: 30 Jul 2013
 *      Author: pjbotma
 */

#include "comms.h"

enum commsState
{
    waitForId,
    waitForData
} uartState,
    i2cState;

typedef struct
{
    uint8_t id;
    uint8_t len;
    uint8_t error;
    uint8_t processed;
    uint8_t params[COMMS_TCMD_PARAMLEN];
} COMMS_TCMD_TypeDef;

COMMS_TCMD_TypeDef tcmdBuffer[COMMS_TCMD_BUFFLEN];

uint8_t tcmdReadIndex;
uint8_t tcmdWriteIndex;
uint8_t tcmdBuffFull;

uint16_t tcmdCount;
uint16_t tlmCount;

uint16_t commsErr;

uint8_t uartRxIndex;
uint8_t uartTxIndex;
uint8_t uartTxBuffer[64];

uint8_t i2cRxIndex;
uint8_t i2cTxIndex;
uint8_t i2cTxBuffer[64];

uint8_t canRxIndex;
uint8_t canRxBuffer[13];
uint8_t canTxIndex;
uint8_t canTxBuffer[13];

uint8_t debugStr[512];
uint16_t debugLen;

// Forward declarations
void addToBuffer_uint8(uint8_t* buffer, uint8_t data);
void addToBuffer_uint16(uint8_t* buffer, uint16_t data);
void addToBuffer_int16(uint8_t* buffer, int16_t data);
void addToBuffer_uint32(uint8_t* buffer, uint32_t data);

/**
 * @author Pieter J. Botma
 * @date   14/01/2013.
 *
 * This function initializes the various communication peripherals and the
 * variables to be used.
 *
 */
void COMMS_Init(void)
{
    int i;

    BSP_UART_Init(BSP_UART_DEBUG);
    BSP_I2C_Init(BSP_I2C_SYS, false);
    BSP_I2C_Init(BSP_I2C_SUB, false);

#if defined(CubeCompV3B)
    BSP_CAN_Init();
#endif

    for (i = 0; i < COMMS_TCMD_BUFFLEN; i++)
    {
        tcmdBuffer[i].error = 0;
        tcmdBuffer[i].processed = 1;
    }

    tcmdReadIndex = 0;
    tcmdWriteIndex = 0;
    tcmdBuffFull = 0;

    tcmdCount = 0;
    tlmCount = 0;

    commsErr = 0x00;

    uartRxIndex = 0;
    uartTxIndex = 0;

    i2cRxIndex = 0;
    i2cTxIndex = 0;

    canRxIndex = 0;
    canTxIndex = 0;

    debugLen = 0;
}

/***************************************************************************/ /**
  * @author Pieter J. Botma
  * @date   14/01/2013.
  *
  * This function processes the supplied telemetry request (id) by adding the
  * relevant data to the supplied buffer (txBuffer).
  *
  * @param [in] id
  *   Supplied id of telemetry request.
  * @param [out] txBuffer
  *   Pointer of buffer that telemetry data should be added to.
  ******************************************************************************/
uint8_t processTLM(uint8_t id, uint8_t* txBuffer)
{
    uint8_t tlmLen;
    uint8_t i;
    uint8_t processed = 1;

    switch (id)
    {
        case 0x80: // status
            addToBuffer_uint32(&(txBuffer[0]), time(NULL));
            addToBuffer_uint8(&(txBuffer[4]), (uint8_t)FIRMWARE_MAJOR);
            addToBuffer_uint8(&(txBuffer[5]), (uint8_t)FIRMWARE_MINOR);
            tlmLen = 6;
            break;

        case 0x81: // communication status
            addToBuffer_uint16(&(txBuffer[0]), tcmdCount);
            addToBuffer_uint16(&(txBuffer[2]), tlmCount);
            addToBuffer_uint16(&(txBuffer[4]), commsErr);
            tlmLen = 6;
            // clear error flags
            commsErr = 0x00;
            break;

        case 0x83: // current time
            addToBuffer_uint32(&(txBuffer[0]), time(NULL));
            addToBuffer_uint16(&(txBuffer[4]), 0);
            tlmLen = 6;
            break;

        case 0x84: // telecommands processed
            // loop through telecommand buffer until unprocessed tcmd found
            for (i = 0; (i < COMMS_TCMD_BUFFLEN) & processed; i++)
                processed &= tcmdBuffer[i].processed;
            addToBuffer_uint8(&(txBuffer[0]), processed);
            tlmLen = 1;
            break;

        case 0x85: // latest adc result
            addToBuffer_uint16(&(txBuffer[0]), BSP_ADC_getData(0));
            addToBuffer_uint16(&(txBuffer[2]), BSP_ADC_getData(1));
            addToBuffer_uint16(&(txBuffer[4]), BSP_ADC_getData(2));
            addToBuffer_uint16(&(txBuffer[6]), BSP_ADC_getData(3));
            addToBuffer_uint16(&(txBuffer[8]), BSP_ADC_getData(TEMPERATURE));
            tlmLen = 10;
            break;

        case 0x86: // latest EBI result
            addToBuffer_uint8(&(txBuffer[0]), testResultEBI[0]);
            addToBuffer_uint8(&(txBuffer[1]), testResultEBI[1]);
            addToBuffer_uint8(&(txBuffer[2]), testResultEBI[2]);
            tlmLen = 3;
            break;

        case 0x87: // latest MicroSD result
            addToBuffer_uint8(&(txBuffer[0]), testResultMicroSD);
            tlmLen = 1;
            break;

        default:
            // error: unknown telemetry id
            // TODO: Define error value
            break;
    }
    tlmCount++;
    return tlmLen;
}

/**
 * @author Pieter J. Botma
 * @date   14/01/2013.
 *
 * This function identifies the supplied telecommand (id) and returns the length
 * of the telecommand.
 *
 * @param [in] id
 *   Supplied id of the telecommand for which the length is required.
 *
 * @return
 *   Returns the length of the specified telecommand.
 */
uint8_t identifyTCMD(uint8_t id)
{
    uint8_t tcmdLen;

    switch (id)
    {
        case 't':
        case 'a':
        case 'e':
        case 'm':
        case 'r':
        case '1':
        case '!':
        case '2':
        case '@':
        case 'l':
        case 'c':
            tcmdLen = 0;
            break;

        default:
            tcmdLen = 0;
            break;
    }

    return tcmdLen;
}

/**
 * @author Pieter J. Botma
 * @date   14/01/2013.
 *
 * This function processes all received telecommands. It loops through the
 * telecommand buffer until no more unprocessed telecommands are pending. If a
 * error occurred during reception, the error is output over debug UART and
 * the error flag is cleared.
 */
void COMMS_processTCMD(void)
{
    // loop through TCMD buffer until all TCMDs have been processed
    while (!(tcmdBuffer[(tcmdReadIndex + 1) % COMMS_TCMD_BUFFLEN].processed))
    {
        tcmdReadIndex = (tcmdReadIndex + 1) % COMMS_TCMD_BUFFLEN;

        // process errors
        if (tcmdBuffer[tcmdReadIndex].error)
        {
            // output error over debug UART
            debugLen = sprintf((char*)debugStr, "\nERROR: %d\n", tcmdBuffer[tcmdReadIndex].error);
            BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);

            // clear error
            tcmdBuffer[tcmdReadIndex].error = 0;
        }
        // process telecommands
        else
        {
            switch (tcmdBuffer[tcmdReadIndex].id)
            {
                case 't':
                    TEST_RTC();
                    break;

                case 'a':
                    TEST_ADC();
                    break;

                case 'e':
                    TEST_EBI();
                    break;

                //			case 'm':
                //				TEST_microSD();
                //				break;

                case 'r':
                    SCB->AIRCR = 0x05FA0004; // Reset MCU
                    break;

                case '1':
                    Sram1EnabledStatus = BSP_EBI_enableSRAM(bspEbiSram1);
                    if (Sram1EnabledStatus)
                    {
                        debugLen = sprintf((char*)debugStr, "\n\nSRAM 1 enabled!");
                        BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
                    }
                    else
                    {
                        debugLen = sprintf((char*)debugStr, "\n\nSRAM 1 could not be enabled!");
                        BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
                    }
                    break;

                case '!':
                    BSP_EBI_disableSRAM(bspEbiSram1);
                    debugLen = sprintf((char*)debugStr, "\n\nSRAM 1 disabled!");
                    BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
                    Sram1EnabledStatus = 0;
                    break;

                case '2':
                    Sram2EnabledStatus = BSP_EBI_enableSRAM(bspEbiSram2);
                    if (Sram2EnabledStatus)
                    {
                        debugLen = sprintf((char*)debugStr, "\n\nSRAM 2 enabled!");
                        BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
                    }
                    else
                    {
                        debugLen = sprintf((char*)debugStr, "\n\nSRAM 2 could not be enabled!");
                        BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
                    }
                    break;

                case '@':
                    BSP_EBI_disableSRAM(bspEbiSram2);
                    debugLen = sprintf((char*)debugStr, "\n\nSRAM 2 disabled!");
                    BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
                    Sram2EnabledStatus = 0;
                    break;

                case 'l':
                    debugLen = sprintf((char*)debugStr,
                        "\n\nSRAM 1 latched: %d \nSRAM 2 latched: %d \nSRAM 1 On: %d \nSRAM 2 On: %d",
                        BSP_SEE_isSramLatched(bspSeeSram1),
                        BSP_SEE_isSramLatched(bspSeeSram2),
                        Sram1EnabledStatus,
                        Sram2EnabledStatus);
                    BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
                    break;

                case 'c':
                    BACKGROUND_displayResetCause();
                    BACKGROUND_displayWelcomeMessage();
                    break;

                case 'w': // test internal watchdog
                    TEST_Wdg(true, false);
                    break;

                case 'W': // test external watchdog
                    TEST_Wdg(false, true);
                    break;

                case 's':
                    TEST_SRAM();
                    break;

                default:
                    debugLen = sprintf((char*)debugStr, "\nERROR: Unknown telecommand ID: %c\n", tcmdBuffer[tcmdReadIndex].id);
                    BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
                    break;
            }

            tcmdCount++;
        }

        tcmdBuffer[tcmdReadIndex].processed = 1;
    }
}

void addToBuffer_uint8(uint8_t* buffer, uint8_t data)
{
    *(buffer) = data;
}

void addToBuffer_uint16(uint8_t* buffer, uint16_t data)
{
    *((uint16_t*)buffer) = data;
}

void addToBuffer_int16(uint8_t* buffer, int16_t data)
{
    *((int16_t*)buffer) = data;
}

void addToBuffer_uint32(uint8_t* buffer, uint32_t data)
{
    *((uint32_t*)buffer) = data;
}

/**
 * UART interrupt handler
 */
void BSP_UART_DEBUG_IRQHandler(void)
{
    uint8_t tempId, tempLen, tempData;

    // disable interrupt
    BSP_UART_DEBUG->IEN &= ~USART_IEN_RXDATAV;

    switch (uartState)
    {
        case waitForId:

            tempId = BSP_UART_DEBUG->RXDATA;

            // Telemetry ID
            if ((tempId & COMMS_ID_TYPE) == COMMS_ID_TLM)
            {
                tempLen = processTLM(tempId, uartTxBuffer);

                if (BSP_UART_txInProgress())
                {
                    commsErr = COMMS_ERROR_UARTTLM;
                }
                else
                {
                    BSP_UART_txBuffer(BSP_UART_DEBUG, uartTxBuffer, tempLen, false);
                }
            }
            // Telecommand ID
            else
            {
                // check for tcmd buffer overflow
                if (tcmdBuffer[(tcmdWriteIndex + 1) % COMMS_TCMD_BUFFLEN].processed == 0)
                {
                    tcmdBuffFull = 1;
                    commsErr = COMMS_ERROR_TCMDBUFOF;
                }
                // add data to new tcmd paramter storage
                else
                {
                    tcmdWriteIndex = (tcmdWriteIndex + 1) % COMMS_TCMD_BUFFLEN;
                    tcmdBuffFull = 0;

                    tcmdBuffer[tcmdWriteIndex].id = tempId;
                    tcmdBuffer[tcmdWriteIndex].len = identifyTCMD(tempId);

                    // if tcmd has length, switch uart state, else flag for processing
                    if (tcmdBuffer[tcmdWriteIndex].len > 0)
                    {
                        uartRxIndex = 0;
                        uartState = waitForData;
                    }
                    else
                    {
                        tcmdBuffer[tcmdWriteIndex].processed = 0;
                    }
                }
            }
            break;

        case waitForData:

            if (!tcmdBuffFull)
            {
                tcmdBuffer[tcmdWriteIndex].params[uartRxIndex++] = BSP_UART_DEBUG->RXDATA;

                if (uartRxIndex == tcmdBuffer[tcmdWriteIndex].len)
                {
                    tcmdBuffer[tcmdWriteIndex].processed = 0;
                    uartState = waitForId;
                }
            }
            else
            {
                // read data to clear interrupt
                tempData = BSP_UART_DEBUG->RXDATA;
            }
            break;

        default:
            break;
    }

    // enable interrupt
    BSP_UART_DEBUG->IEN |= USART_IEN_RXDATAV;
}

/**
 * I2C interrupt handler
 */
void I2C0_IRQHandler(void)
{
    int status;
    uint8_t tempAddr, tempId, tempData;

    status = BSP_I2C_SYS->IF;

    switch (i2cState)
    {
        case waitForId:

            if (status & I2C_IF_ADDR)
            {
                I2C_IntClear(BSP_I2C_SYS, I2C_IFC_ADDR);

                tempAddr = BSP_I2C_SYS->RXDATA;

                // should not receive read request while waiting for ID
                if ((tempAddr & COMMS_I2C_TYPE) == COMMS_I2C_READ)
                {
                    commsErr = COMMS_ERROR_I2CTLM;

                    // send previous TLM already in buffer
                    i2cState = waitForData;
                    i2cTxIndex = 0;
                    BSP_I2C_SYS->TXDATA = i2cTxBuffer[i2cTxIndex++];
                }
            }
            else if (status & I2C_IF_RXDATAV)
            {
                tempId = BSP_I2C_SYS->RXDATA;

                // Telemetry ID
                if ((tempId & COMMS_ID_TYPE) == COMMS_ID_TLM)
                {
                    processTLM(tempId, i2cTxBuffer);

                    i2cTxIndex = 0;
                    i2cState = waitForData;
                }
                // Telecommand ID
                else
                {
                    // check for tcmd buffer overflow
                    if (tcmdBuffer[(tcmdWriteIndex + 1) % COMMS_TCMD_BUFFLEN].processed == 0)
                    {
                        tcmdBuffFull = 1;
                        commsErr = COMMS_ERROR_TCMDBUFOF;

                        i2cState = waitForData;
                    }
                    else
                    {
                        tcmdWriteIndex = (tcmdWriteIndex + 1) % COMMS_TCMD_BUFFLEN;
                        tcmdBuffFull = 0;

                        tcmdBuffer[tcmdWriteIndex].id = tempId;
                        tcmdBuffer[tcmdWriteIndex].len = identifyTCMD(tempId);

                        i2cRxIndex = 0;
                        i2cState = waitForData;
                    }
                }
            }
            else if (status & I2C_IEN_SSTOP)
            {
                I2C_IntClear(BSP_I2C_SYS, I2C_IEN_SSTOP);
            }
            break;

        case waitForData:

            // if data received which is address, start of TLM request
            if (status & I2C_IF_ADDR)
            {
                I2C_IntClear(BSP_I2C_SYS, I2C_IFC_ADDR);

                tempAddr = BSP_I2C_SYS->RXDATA;

                // Send first byte of telemetry buffer
                if ((tempAddr & COMMS_I2C_TYPE) == COMMS_I2C_READ)
                {
                    BSP_I2C_SYS->TXDATA = i2cTxBuffer[i2cTxIndex++];
                }
            }
            // if data received which is not address, TCMD data
            else if (status & I2C_IF_RXDATAV)
            {
                if (tcmdBuffFull)
                {
                    // read data to clear interrupt
                    tempData = BSP_I2C_SYS->RXDATA;
                }
                else if (i2cRxIndex < COMMS_TCMD_PARAMLEN)
                {
                    // save data in TCMD buffer
                    tcmdBuffer[tcmdWriteIndex].params[i2cRxIndex++] = BSP_I2C_SYS->RXDATA;
                }
                else
                {
                    // read data to clear interrupt
                    tempData = BSP_I2C_SYS->RXDATA;

                    // increment index counter for error checking when stop received
                    i2cRxIndex++;
                }
            }

            // if ACK received, another TLM byte requested
            if (status & I2C_IF_ACK)
            {
                I2C_IntClear(BSP_I2C_SYS, I2C_IEN_ACK);

                BSP_I2C_SYS->TXDATA = i2cTxBuffer[i2cTxIndex++];
            }

            // if NACK received, tlm transmission ends
            if (status & I2C_IF_NACK)
            {
                I2C_IntClear(BSP_I2C_SYS, I2C_IFC_NACK);

                i2cState = waitForId;
            }

            // if STOP received, tcmd transmission ends
            if (status & I2C_IF_SSTOP)
            {
                I2C_IntClear(BSP_I2C_SYS, I2C_IFC_SSTOP);

                if (i2cRxIndex == tcmdBuffer[tcmdWriteIndex].len)
                {
                    // flag message to be processed
                    tcmdBuffer[tcmdWriteIndex].processed = 0;
                }
                else if (i2cRxIndex < tcmdBuffer[tcmdWriteIndex].len)
                {
                    tcmdBuffer[tcmdWriteIndex].error = COMMS_TCMDERR_PARAMUF;
                    tcmdBuffer[tcmdWriteIndex].processed = 0;
                }
                else
                {
                    tcmdBuffer[tcmdWriteIndex].error = COMMS_TCMDERR_PARAMOF;
                    tcmdBuffer[tcmdWriteIndex].processed = 0;
                }

                // telecommand data reception complete
                i2cState = waitForId;
            }
            break;
    }
}

/**
 * I2C interrupt handler
 */
void I2C1_IRQHandler(void)
{
    int status;
    uint8_t tempAddr, tempId, tempData;

    status = BSP_I2C_SUB->IF;

    switch (i2cState)
    {
        case waitForId:

            if (status & I2C_IF_ADDR)
            {
                I2C_IntClear(BSP_I2C_SUB, I2C_IFC_ADDR);

                tempAddr = BSP_I2C_SUB->RXDATA;

                // should not receive read request while waiting for ID
                if ((tempAddr & COMMS_I2C_TYPE) == COMMS_I2C_READ)
                {
                    commsErr = COMMS_ERROR_I2CTLM;

                    // send previous TLM already in buffer
                    i2cState = waitForData;
                    i2cTxIndex = 0;
                    BSP_I2C_SUB->TXDATA = i2cTxBuffer[i2cTxIndex++];
                }
            }
            else if (status & I2C_IF_RXDATAV)
            {
                tempId = BSP_I2C_SUB->RXDATA;

                // Telemetry ID
                if ((tempId & COMMS_ID_TYPE) == COMMS_ID_TLM)
                {
                    processTLM(tempId, i2cTxBuffer);

                    i2cTxIndex = 0;
                    i2cState = waitForData;
                }
                // Telecommand ID
                else
                {
                    // check for tcmd buffer overflow
                    if (tcmdBuffer[(tcmdWriteIndex + 1) % COMMS_TCMD_BUFFLEN].processed == 0)
                    {
                        tcmdBuffFull = 1;
                        commsErr = COMMS_ERROR_TCMDBUFOF;

                        i2cState = waitForData;
                    }
                    else
                    {
                        tcmdWriteIndex = (tcmdWriteIndex + 1) % COMMS_TCMD_BUFFLEN;
                        tcmdBuffFull = 0;

                        tcmdBuffer[tcmdWriteIndex].id = tempId;
                        tcmdBuffer[tcmdWriteIndex].len = identifyTCMD(tempId);

                        i2cRxIndex = 0;
                        i2cState = waitForData;
                    }
                }
            }
            else if (status & I2C_IEN_SSTOP)
            {
                I2C_IntClear(BSP_I2C_SUB, I2C_IEN_SSTOP);
            }
            break;

        case waitForData:

            // if data received which is address, start of TLM request
            if (status & I2C_IF_ADDR)
            {
                I2C_IntClear(BSP_I2C_SUB, I2C_IFC_ADDR);

                tempAddr = BSP_I2C_SUB->RXDATA;

                // Send first byte of telemetry buffer
                if ((tempAddr & COMMS_I2C_TYPE) == COMMS_I2C_READ)
                {
                    BSP_I2C_SUB->TXDATA = i2cTxBuffer[i2cTxIndex++];
                }
            }
            // if data received which is not address, TCMD data
            else if (status & I2C_IF_RXDATAV)
            {
                if (tcmdBuffFull)
                {
                    // read data to clear interrupt
                    tempData = BSP_I2C_SUB->RXDATA;
                }
                else if (i2cRxIndex < COMMS_TCMD_PARAMLEN)
                {
                    // save data in TCMD buffer
                    tcmdBuffer[tcmdWriteIndex].params[i2cRxIndex++] = BSP_I2C_SUB->RXDATA;
                }
                else
                {
                    // read data to clear interrupt
                    tempData = BSP_I2C_SUB->RXDATA;

                    // increment index counter for error checking when stop received
                    i2cRxIndex++;
                }
            }

            // if ACK received, another TLM byte requested
            if (status & I2C_IF_ACK)
            {
                I2C_IntClear(BSP_I2C_SUB, I2C_IEN_ACK);

                BSP_I2C_SUB->TXDATA = i2cTxBuffer[i2cTxIndex++];
            }

            // if NACK received, tlm transmission ends
            if (status & I2C_IF_NACK)
            {
                I2C_IntClear(BSP_I2C_SUB, I2C_IFC_NACK);

                i2cState = waitForId;
            }

            // if STOP received, tcmd transmission ends
            if (status & I2C_IF_SSTOP)
            {
                I2C_IntClear(BSP_I2C_SUB, I2C_IFC_SSTOP);

                if (i2cRxIndex == tcmdBuffer[tcmdWriteIndex].len)
                {
                    // flag message to be processed
                    tcmdBuffer[tcmdWriteIndex].processed = 0;
                }
                else if (i2cRxIndex < tcmdBuffer[tcmdWriteIndex].len)
                {
                    tcmdBuffer[tcmdWriteIndex].error = COMMS_TCMDERR_PARAMUF;
                    tcmdBuffer[tcmdWriteIndex].processed = 0;
                }
                else
                {
                    tcmdBuffer[tcmdWriteIndex].error = COMMS_TCMDERR_PARAMOF;
                    tcmdBuffer[tcmdWriteIndex].processed = 0;
                }

                // telecommand data reception complete
                i2cState = waitForId;
            }
            break;
    }
}

/*
 * CAN interrupt handler
 */
void GPIO_ODD_IRQHandler(void)
{
    // disable interrupt
    NVIC_DisableIRQ(GPIO_ODD_IRQn);

    BSP_CAN_readRxBuffer(bspCanRx0Id, canRxBuffer);

    uint8_t tempId, tempLen, i;

    tempId = (uint8_t)BSP_CAN_reg2Id(canRxBuffer[0], canRxBuffer[1]);

    // Telemetry ID
    if ((tempId & COMMS_ID_TYPE) == COMMS_ID_TLM)
    {
        tempLen = processTLM(tempId, &(canTxBuffer[5]));

        canTxBuffer[0] = 0x00;    // TXB0SIDH
        canTxBuffer[1] = 0x20;    // TXB0SIDL
        canTxBuffer[4] = tempLen; // TXB0DLC

        // Maybe do error checking here! (If tx not in process?)

        BSP_CAN_writeTxBuffer(bspCanTx0Id, canTxBuffer);
        BSP_CAN_requestToSend(bspCanRtsTx0);
    }
    // Telecommand ID
    else
    {
        // check for tcmd buffer overflow
        if (tcmdBuffer[(tcmdWriteIndex + 1) % COMMS_TCMD_BUFFLEN].processed == 0)
        {
            tcmdBuffFull = 1;
            commsErr = COMMS_ERROR_TCMDBUFOF;
        }
        // add data to tcmd buffer
        else
        {
            tcmdWriteIndex = (tcmdWriteIndex + 1) % COMMS_TCMD_BUFFLEN;
            tcmdBuffFull = 0;

            tcmdBuffer[tcmdWriteIndex].id = tempId;
            tcmdBuffer[tcmdWriteIndex].len = identifyTCMD(tempId);

            for (i = 0; i < tcmdBuffer[tcmdWriteIndex].len; i++)
            {
                tcmdBuffer[tcmdWriteIndex].params[i] = canRxBuffer[i];
            }

            tcmdBuffer[tcmdWriteIndex].processed = 0;
        }
    }
    // Clear interrupts
    GPIO_IntClear(_GPIO_IFC_MASK & 0xFFFF);
    // Enable interrupt
    NVIC_EnableIRQ(GPIO_ODD_IRQn);
}
