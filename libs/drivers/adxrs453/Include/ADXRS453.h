#ifndef __ADXRS453_H__
#define __ADXRS453_H__

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
char ADXRS453_Init(void);

/*! Reads the value of a register. */
unsigned short ADXRS453_GetRegisterValue(unsigned char registerAddress);

/*! Writes data into a register. */
void ADXRS453_SetRegisterValue(unsigned char registerAddress,
                               unsigned short registerValue);

/*! Reads the sensor data. */
unsigned long ADXRS453_GetSensorData(void);

/*! Reads the rate data and converts it to degrees/second. */
float ADXRS453_GetRate(void);

/*! Reads the temperature sensor data and converts it to degrees Celsius. */
float ADXRS453_GetTemperature(void);

#endif // __ADXRS453_H__
