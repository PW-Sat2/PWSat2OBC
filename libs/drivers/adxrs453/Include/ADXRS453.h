#ifndef __ADXRS453_H__
#define __ADXRS453_H__
#include "io_map.h"
#include "spidrv.h"
/******************************************************************************/
/************************** ADXRS453 Definitions ******************************/
/******************************************************************************/

#define ADXRS453_SLAVE_ID       1

#define ADXRS453_READ           (1 << 7)
#define ADXRS453_WRITE          (1 << 6)
#define ADXRS453_SENSOR_DATA    (1 << 5)

#define ADXRS453_REG_RATE       0x00
#define ADXRS453_REG_TEM        0x02
#define ADXRS453_REG_LOCST      0x04
#define ADXRS453_REG_HICST      0x06
#define ADXRS453_REG_QUAD       0x08
#define ADXRS453_REG_FAULT      0x0A
#define ADXRS453_REG_PID        0x0C
#define ADXRS453_REG_SN_HIGH    0x0E
#define ADXRS453_REG_SN_LOW     0x10

/******************************************************************************/
/************************ Functions Declarations ******************************/
/******************************************************************************/

/*! Initializes the ADXRS453 and checks if the device is present. */

#define ADXRS453_SPI                                              \
{                                                                         \
  ADXRS453_SPI_USART_PORT,                       /* USART port                       */    \
  ADXRS453_SPI_USART_ROUTE_LOCATION,   /* USART pins location number ||  MOSI-PD0, MISO-PD1, CLK-PD2, CS-PD3       */    \
  1000000,                      /* Bitrate                          */    \
  8,                            /* Frame length                     */    \
  0,                            /* Dummy tx value for rx only funcs */    \
  spidrvMaster,                 /* SPI mode                         */    \
  spidrvBitOrderMsbFirst,       /* Bit order on bus                 */    \
  spidrvClockMode0,             /* SPI clock/phase mode             */    \
  spidrvCsControlApplication,          /* CS controlled by the driver      */    \
  spidrvSlaveStartDelayed          \
}


typedef struct ADXRS453_Init {
	uint8_t             csPortLocation;
	uint8_t             csPinLocation;
	uint8_t				gyroNumber;
} ADXRS453_Init_t;

void ADXRS453Spi_Init(void);

char ADXRS453_Init(ADXRS453_Init_t *gyro);

void SPISendB(ADXRS453_Init_t *gyro,SPIDRV_Handle_t 	handle,
		const void * 	buffer,
		int 	count );
void SPIRecvB(ADXRS453_Init_t *gyro,SPIDRV_Handle_t 	handle,
		void * 	buffer,
		int 	count );
/*! Reads the value of a register. */
unsigned short ADXRS453_GetRegisterValue(ADXRS453_Init_t *gyro,unsigned char registerAddress);

/*! Writes data into a register. */
void ADXRS453_SetRegisterValue(ADXRS453_Init_t *gyro,unsigned char registerAddress,
                               unsigned short registerValue);

/*! Reads the sensor data. */
unsigned long ADXRS453_GetSensorData(ADXRS453_Init_t *gyro);

/*! Reads the rate data and converts it to degrees/second. */
float ADXRS453_GetRate(ADXRS453_Init_t *gyro);

/*! Reads the temperature sensor data and converts it to degrees Celsius. */
float ADXRS453_GetTemperature(ADXRS453_Init_t *gyro);

#endif // __ADXRS453_H__
