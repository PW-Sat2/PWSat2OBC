
#include "ADXRS453.h"       // ADXRS453 definitions
#include "spidrv.h"
#include "FreeRTOS.h"
#include "drivers/swo.h"
#include "io_map.h"
#include <em_gpio.h>
SPIDRV_HandleData_t handleData;
SPIDRV_Handle_t handle = &handleData;
bool SPIInitialized=false;

void SPISendB(ADXRS453_Init_t *gyro , SPIDRV_Handle_t 	handle,
		const void * 	buffer,
		int 	count )
{
	GPIO_PinOutClear((GPIO_Port_TypeDef)gyro->csPortLocation,gyro->csPinLocation);
    SPIDRV_MTransmitB( handle,buffer,count);
	GPIO_PinOutSet((GPIO_Port_TypeDef)gyro->csPortLocation,gyro->csPinLocation);


}
void SPIRecvB(ADXRS453_Init_t *gyro,SPIDRV_Handle_t 	handle,
		void * 	buffer,
		int 	count )
{
	GPIO_PinOutClear((GPIO_Port_TypeDef)gyro->csPortLocation,gyro->csPinLocation);
	SPIDRV_MReceiveB(handle,buffer,count);
	GPIO_PinOutSet((GPIO_Port_TypeDef)gyro->csPortLocation,gyro->csPinLocation);
}
void ADXRS453Spi_Init() {
	/* Initialize the SPI communication peripheral  */
	if(SPIInitialized==false){
	 SPIDRV_Init_t initData = ADXRS453_SPI;
	// Initialize a SPI driver instance
	 SPIDRV_Init( handle, &initData );
	 SPIInitialized=true;
	}

}
char ADXRS453_Init(ADXRS453_Init_t *gyro)
{
	unsigned char  dataBuffer[4] = {0x20, 0x00, 0x00, 0x03};
    char           status     = 0;
    unsigned short adxrs453Id;

    ADXRSSpi_Init();
    GPIO_PinModeSet( (GPIO_Port_TypeDef)gyro->csPortLocation, gyro->csPinLocation ,gpioModePushPull, 1 );
    GPIO_PinOutSet((GPIO_Port_TypeDef)gyro->csPortLocation,gyro->csPinLocation);
//RECOMMENDED START-UP SEQUENCE WITH CHK BIT ASSERTION see datasheet
    vTaskDelay(100 / portTICK_PERIOD_MS);
    SPISendB(gyro, handle, dataBuffer, 4 );
    vTaskDelay(50 / portTICK_PERIOD_MS);
    dataBuffer[3]=0x00;
    SPISendB(gyro, handle, dataBuffer, 4 );
    vTaskDelay(50 / portTICK_PERIOD_MS);
    SPISendB(gyro, handle, dataBuffer, 4 );
    vTaskDelay(50 / portTICK_PERIOD_MS);
    SPISendB(gyro, handle, dataBuffer, 4 );
    vTaskDelay(50 / portTICK_PERIOD_MS);
    SPISendB(gyro, handle, dataBuffer, 4 );
    vTaskDelay(50 / portTICK_PERIOD_MS);
    //CHECK IF ADXRS IS STARTUPED
    adxrs453Id = ADXRS453_GetRegisterValue(gyro,ADXRS453_REG_PID);
    if((adxrs453Id >> 8) != 0x52)
    {
        status = -1;
    }
    
    return status;
}


void ADXRS453_DeInit(void){
	SPIDRV_DeInit( handle);
	SPIInitialized=false;
}


unsigned short ADXRS453_GetRegisterValue(ADXRS453_Init_t *gyro, unsigned char registerAddress)
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

    SPISendB(gyro, handle, sendBuffer, 4 );
    vTaskDelay(50 / portTICK_PERIOD_MS);
    SPIRecvB(gyro, handle, recvBuffer, 4);
    vTaskDelay(50 / portTICK_PERIOD_MS);

    registerValue = ((unsigned short)recvBuffer[1] << 11) |
                    ((unsigned short)recvBuffer[2] << 3) |
                    (recvBuffer[3] >> 5);

    return registerValue;
}

void ADXRS453_SetRegisterValue(ADXRS453_Init_t *gyro,unsigned char registerAddress,
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
    SPISendB(gyro, handle, sendBuffer, 4 );
    vTaskDelay(50 / portTICK_PERIOD_MS);

}

unsigned long ADXRS453_GetSensorData(ADXRS453_Init_t *gyro)
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
    SPISendB(gyro, handle, sendBuffer, 4 );
    vTaskDelay(50 / portTICK_PERIOD_MS);
    SPIRecvB(gyro, handle, recvBuffer, 4 );
    vTaskDelay(50 / portTICK_PERIOD_MS);
    registerValue = ((unsigned long)recvBuffer[0] << 24) |
                    ((unsigned long)recvBuffer[1] << 16) |
                    ((unsigned short)recvBuffer[2] << 8) |
                    recvBuffer[3];
    
    return registerValue;
}

float ADXRS453_GetRate(ADXRS453_Init_t *gyro)
{
    unsigned short registerValue = 0;
    float          rate          = 0.0;
    
    registerValue = ADXRS453_GetRegisterValue(gyro,ADXRS453_REG_RATE);
   
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

float ADXRS453_GetTemperature(ADXRS453_Init_t *gyro)
{
    unsigned long registerValue = 0;
    float          temperature   = 0;
    
    registerValue = ADXRS453_GetRegisterValue(gyro,ADXRS453_REG_TEM);
    registerValue = (registerValue >> 6) - 0x31F;
    temperature = (float) registerValue / 5;

    return temperature;
}
