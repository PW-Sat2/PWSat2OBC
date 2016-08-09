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
typedef struct SPI_TransferPairResultCode
{
	Ecode_t resultCodeWrite;
	Ecode_t resultCodeRead;
} SPI_TransferPairResultCode_t;
typedef struct ADXRS453_PinLocations {
	uint8_t             csPortLocation;
	uint8_t             csPinLocation;
} ADXRS453_PinLocations_t;
typedef Ecode_t (*GyroSPIWriteProcedure)(ADXRS453_PinLocations_t *locations,SPIDRV_Handle_t 	handle, const void * buffer, uint8_t length);
typedef SPI_TransferPairResultCode_t (*GyroSPIWriteReadProcedure)(ADXRS453_PinLocations_t *locations,SPIDRV_Handle_t 	handle, const void * buffer, uint8_t length);


typedef struct
{
	GyroSPIWriteProcedure writeProc;
	GyroSPIWriteReadProcedure readProc;
} GyroInterface_t;

typedef struct ADXRS453_Obj {
	ADXRS453_PinLocations_t pinLocations;
	GyroInterface_t interface;
} ADXRS453_Obj_t;

typedef struct SPI_TransferReturn
{
	SPI_TransferPairResultCode_t resultCodes;
	union
	{
		int16_t sensorResult;
		uint32_t dataResult;
	}result;
} SPI_TransferReturn_t;






void ADXRS453Spi_Init(ADXRS453_Obj_t *gyro);

void ADXRS453_Init(ADXRS453_Obj_t *gyro,
		SPIDRV_Handle_t 	handle);

Ecode_t SPISendB(ADXRS453_PinLocations_t *locations,
		SPIDRV_Handle_t 	handle,
		const void * 	buffer,
		uint8_t 	length );

Ecode_t SPIRecvB(ADXRS453_PinLocations_t *locations,
		SPIDRV_Handle_t 	handle,
		void * 	buffer,
		uint8_t 	length );
SPI_TransferPairResultCode_t SPISendRecvB(ADXRS453_PinLocations_t *locations,
		SPIDRV_Handle_t 	handle,
		void * 	buffer,
		uint8_t 	length );
/*! Reads the value of a register. */
SPI_TransferReturn_t ADXRS453_GetRegisterValue(ADXRS453_Obj_t *gyro,
									SPIDRV_Handle_t handle,
									uint8_t registerAddress);

/*! Writes data into a register. */
SPI_TransferReturn_t ADXRS453_SetRegisterValue(ADXRS453_Obj_t *gyro,
								SPIDRV_Handle_t 	handle,
								uint8_t registerAddress,
								uint16_t registerValue);

/*! Reads the sensor data. */
SPI_TransferReturn_t ADXRS453_GetSensorData(ADXRS453_Obj_t *gyro,
		SPIDRV_Handle_t 	handle);

/*! Reads the rate data and converts it to degrees/second. */
SPI_TransferReturn_t ADXRS453_GetRate(ADXRS453_Obj_t *gyro,
		SPIDRV_Handle_t 	handle);

/*! Reads the temperature sensor data and converts it to degrees Celsius. */
SPI_TransferReturn_t  ADXRS453_GetTemperature(ADXRS453_Obj_t *gyro,
		SPIDRV_Handle_t 	handle);



#endif // __ADXRS453_H__
