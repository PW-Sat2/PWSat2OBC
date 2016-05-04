/***************************************************************************//**
 *   @file   ADXRS453.c
 *   @brief  Implementation of ADXRS453 Driver.
 *   @author DBogdan (dragos.bogdan@analog.com)
********************************************************************************
 * Copyright 2013(c) Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  - Neither the name of Analog Devices, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *  - The use of this software may or may not infringe the patent rights
 *    of one or more patent holders.  This license does not release you
 *    from the requirement that you obtain separate licenses from these
 *    patent holders to use this software.
 *  - Use of the software either in source or binary form, must be run
 *    on or directly connected to an Analog Devices Inc. component.
 *
 * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include "ADXRS453.h"       // ADXRS453 definitions
#include "spidrv.h"  // Communication definitions
#include "FreeRTOS.h"
#include "drivers/swo.h"
SPIDRV_HandleData_t handleData;
SPIDRV_Handle_t handle = &handleData;


//callbacks
void TransferComplete( SPIDRV_Handle_t handle,
                       Ecode_t transferStatus,
                       int itemsTransferred )
{
  if ( transferStatus == ECODE_EMDRV_SPIDRV_OK )
  {

  }
}

void ReceiveComplete( SPIDRV_Handle_t handle,
                       Ecode_t transferStatus,
                       int itemsTransferred )
{


  if ( transferStatus == ECODE_EMDRV_SPIDRV_OK )
  {

  }
}




/***************************************************************************//**
 * @brief Initializes the ADXRS453 and SPI and checks if the device is present.
 *
 * @return status - Result of the initialization procedure.
 *                  Example:  0 - if initialization was successful (ID starts
 *                                with 0x52).
 *                           -1 - if initialization was unsuccessful.
*******************************************************************************/
char ADXRS453_Init(void)
{
	unsigned char  dataBuffer[4] = {0x20, 0x00, 0x00, 0x03};
    char           status     = 0;
    unsigned short adxrs453Id = 0;
    /* Initialize the SPI communication peripheral  */
    SPIDRV_Init_t initData = SPIDRV_MASTER_USART1;
    initData.slaveStartMode = spidrvSlaveStartDelayed;
      // Initialize a SPI driver instance
    SPIDRV_Init( handle, &initData );
//RECOMMENDED START-UP SEQUENCE WITH CHK BIT ASSERTION see datasheet
    RTCDRV_Delay(100);
    SPIDRV_MTransmit( handle, dataBuffer, 4, TransferComplete );
    RTCDRV_Delay(50);
    dataBuffer[3]=0x00;
    SPIDRV_MTransmit( handle, dataBuffer, 4, TransferComplete );
    RTCDRV_Delay(50);
    SPIDRV_MTransmit( handle, dataBuffer, 4, TransferComplete );
    RTCDRV_Delay(50);
    SPIDRV_MTransmit( handle, dataBuffer, 4, TransferComplete );
    RTCDRV_Delay(50);
    SPIDRV_MTransmit( handle, dataBuffer, 4, TransferComplete );
    RTCDRV_Delay(50);

    adxrs453Id = ADXRS453_GetRegisterValue(ADXRS453_REG_PID);
    if((adxrs453Id >> 8) != 0x52)
    {
        status = -1;
    }
    
    return status;
}

/***************************************************************************//**
 * @brief Deinitializes the SPI.
 *
 * @param None.
 *
 * @return None.
*******************************************************************************/
void ADXRS453_DeInit(void){
	SPIDRV_DeInit( handle);
}

/***************************************************************************//**
 * @brief Reads the value of a register.
 *
 * @param registerAddress - Address of the register.
 *
 * @return registerValue - Value of the register.
*******************************************************************************/
unsigned short ADXRS453_GetRegisterValue(unsigned char registerAddress)
{
    unsigned char  sendBuffer[4] = {0, 0, 0, 0};
    unsigned char  recvBuffer[4] = {0, 0, 0, 0};

    unsigned long  command       = 0;
    unsigned char  bitNo         = 0;
    unsigned char  sum           = 0;
    unsigned short registerValue = 0;

    sendBuffer[0] = ADXRS453_READ | (registerAddress >> 7);
    sendBuffer[1] = (registerAddress << 1);
    command = ((unsigned long)sendBuffer[0] << 24) |
              ((unsigned long)sendBuffer[1] << 16) |
              ((unsigned short)sendBuffer[2] << 8) |
              sendBuffer[3];
    for(bitNo = 31; bitNo > 0; bitNo--)
    {
        sum += ((command >> bitNo) & 0x1);
    }
    if(!(sum % 2))
    {
    	sendBuffer[3] |= 1;
    }

    SPIDRV_MTransmit( handle, sendBuffer, 4,TransferComplete );
    RTCDRV_Delay(50);
    SPIDRV_MReceive( handle, recvBuffer, 4,ReceiveComplete);
    RTCDRV_Delay(50);

    registerValue = ((unsigned short)recvBuffer[1] << 11) |
                    ((unsigned short)recvBuffer[2] << 3) |
                    (recvBuffer[3] >> 5);

    return registerValue;
}

/***************************************************************************//**
 * @brief Writes data into a register.
 *
 * @param registerAddress - Address of the register.
 * @param registerValue - Data value to write.
 *
 * @return None.
*******************************************************************************/
void ADXRS453_SetRegisterValue(unsigned char registerAddress,
                               unsigned short registerValue)
{
	unsigned char  sendBuffer[4] = {0, 0, 0, 0};
    unsigned long command       = 0;
    unsigned char bitNo         = 0;
    unsigned char sum           = 0;
    
    sendBuffer[0] = ADXRS453_WRITE | (registerAddress >> 7);
    sendBuffer[1] = (registerAddress << 1) |
                    (registerValue >> 15);
    sendBuffer[2] = (registerValue >> 7);
    sendBuffer[3] = (registerValue << 1);
    
    command = ((unsigned long)sendBuffer[0] << 24) |
              ((unsigned long)sendBuffer[1] << 16) |
              ((unsigned short)sendBuffer[2] << 8) |
              sendBuffer[3];
    for(bitNo = 31; bitNo > 0; bitNo--)
    {
        sum += ((command >> bitNo) & 0x1);
    }
    if(!(sum % 2))
    {
    	sendBuffer[3] |= 1;
    }
    SPIDRV_MTransmit( handle, sendBuffer, 4, TransferComplete );
    RTCDRV_Delay(50);

}

/***************************************************************************//**
 * @brief Reads the sensor data.
 *
 * @param None.
 *
 * @return registerValue - The sensor data.
*******************************************************************************/
unsigned long ADXRS453_GetSensorData(void)
{
	unsigned char  sendBuffer[4] = {0, 0, 0, 0};
	unsigned char  recvBuffer[4] = {0, 0, 0, 0};
    unsigned long command       = 0;
    unsigned char bitNo         = 0;
    unsigned char sum           = 0;
    unsigned long registerValue = 0;
    sendBuffer[0] = ADXRS453_SENSOR_DATA;
    command = ((unsigned long)sendBuffer[0] << 24) |
              ((unsigned long)sendBuffer[1] << 16) |
              ((unsigned short)sendBuffer[2] << 8) |
              sendBuffer[3];
    for(bitNo = 31; bitNo > 0; bitNo--)
    {
        sum += ((command >> bitNo) & 0x1);
    }
    if(!(sum % 2))
    {
    	sendBuffer[3] |= 1;
    }
    SPIDRV_MTransmit( handle, sendBuffer, 4,TransferComplete );
    RTCDRV_Delay(50);
    SPIDRV_MReceive( handle, recvBuffer, 4, ReceiveComplete );
    RTCDRV_Delay(50);
    registerValue = ((unsigned long)recvBuffer[0] << 24) |
                    ((unsigned long)recvBuffer[1] << 16) |
                    ((unsigned short)recvBuffer[2] << 8) |
                    recvBuffer[3];
    
    return registerValue;
}

/***************************************************************************//**
 * @brief Reads the rate data and converts it to degrees/second.
 *
 * @param None.
 *
 * @return rate - The rate value in degrees/second.
*******************************************************************************/
float ADXRS453_GetRate(void)
{
    unsigned short registerValue = 0;
    float          rate          = 0.0;
    
    registerValue = ADXRS453_GetRegisterValue(ADXRS453_REG_RATE);
   
    /*!< If data received is in positive degree range */
    if(registerValue < 0x8000)
    {
        rate = ((float)registerValue / 80);
    }
    /*!< If data received is in negative degree range */
    else
    {
        rate = (-1) * ((float)(0xFFFF - registerValue + 1) / 80.0);
    }
    
    return rate;
}

/***************************************************************************//**
 * @brief Reads the temperature sensor data and converts it to degrees Celsius.
 *
 * @param None.
 *
 * @return temperature - The temperature value in degrees Celsius.
*******************************************************************************/
float ADXRS453_GetTemperature(void)
{
    unsigned long registerValue = 0;
    float          temperature   = 0;
    
    registerValue = ADXRS453_GetRegisterValue(ADXRS453_REG_TEM);
    registerValue = (registerValue >> 6) - 0x31F;
    temperature = (float) registerValue / 5;
    
    return temperature;
}
