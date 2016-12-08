#ifndef __ADXRS453_H__
#define __ADXRS453_H__
#include "io_map.h"
#include "spidrv.h"
#ifdef __cplusplus
extern "C" {
#endif
/**
 * @defgroup ADXRS453SPIDRIVER Low Level Adxrs453 gyroscope Module Driver
 * @ingroup device_drivers
 *
 * @brief This module is a low level adxrs453 spi module driver.
 *
 * This driver ensures is responsible for
 *  - direct communication with the hardware,
 *  - hardware state management -  initialization procedue, updating current driver configuration,
 *  - for best performance comunication with spi using DMA channel
 *
 *  @{
 */

/**
 * @brief Byte used to generate read command
 */
#define ADXRS453_READ (1 << 7)
/**
 * @brief Byte used to generate write command
 */
#define ADXRS453_WRITE (1 << 6)
/**
 * @brief Byte used to generate sensor data command
 */
#define ADXRS453_SENSOR_DATA (1 << 5)
/**
 * @brief Byte used to generate registers address
 */
#define ADXRS453_REG_RATE 0x00
/**
 * @brief Byte used to generate temperature registers address
 */
#define ADXRS453_REG_TEM 0x02
/**
 * @brief Byte used to generate lo cst registers address
 */
#define ADXRS453_REG_LOCST 0x04
/**
 * @brief Byte used to generate lo cst registers address
 */
#define ADXRS453_REG_HICST 0x06
/**
 * @brief Byte used to generate quad memory registers address
 */
#define ADXRS453_REG_QUAD 0x08
/**
 * @brief Byte used to generate fault registers address
 */
#define ADXRS453_REG_FAULT 0x0A
/**
 * @brief Byte used to generate part id registers address
 */
#define ADXRS453_REG_PID 0x0C
/**
 * @brief Byte used to generate lo serial number registers address
 */
#define ADXRS453_REG_SN_HIGH 0x0E
/**
 * @brief Byte used to generate hi serial number registers address
 */
#define ADXRS453_REG_SN_LOW 0x10

/**
 * @brief SPI setup definition for adxrs453 modules
 */

#define ADXRS453_SPI                                                                                                                       \
    \
{                                                                                                                                   \
        ADXRS453_SPI_USART_PORT,               /* USART port                       */                                                      \
            ADXRS453_SPI_USART_ROUTE_LOCATION, /* USART pins location number ||  MOSI-PD0, MISO-PD1, CLK-PD2, CS-PD3       */              \
            1000000,                           /* Bitrate                          */                                                      \
            8,                                 /* Frame length                     */                                                      \
            0,                                 /* Dummy tx value for rx only funcs */                                                      \
            spidrvMaster,                      /* SPI mode                         */                                                      \
            spidrvBitOrderMsbFirst,            /* Bit order on bus                 */                                                      \
            spidrvClockMode0,                  /* SPI clock/phase mode             */                                                      \
            spidrvCsControlApplication,        /* CS controlled by the driver      */                                                      \
            spidrvSlaveStartDelayed                                                                                                        \
    \
}

/**
 * @brief This type contains pair of error codes for spi transmit communication .
 */

typedef struct SPI_TransferPairResultCode
{
    Ecode_t resultCodeWrite;
    Ecode_t resultCodeRead;
} SPI_TransferPairResultCode_t;

/**
 * @brief This type contains location for chip sellect port and pin .
 */

typedef struct ADXRS453_PinLocations
{
    uint8_t csPortLocation;
    uint8_t csPinLocation;
} ADXRS453_PinLocations_t;

/**
 * @brief Type of pointer to procedure that provides means of sending arbitrary buffer to a device with
 * the SPI bus.
 *
 * This method should be block the execution until entire requested block has been sent.
 * @param[in] pointer for locations structure contains pin and port location for device chip sellect port.
 * @param[in] handle for SPI driver.
 * @param[in] buffer Pointer to buffer that contains the data that should be send to the device.
 * @param[in] length Size of the data stored inside the buffer buffer in bytes.
 * @return sending error code see Ecode_t.
 */
typedef Ecode_t (*GyroSPIWriteProcedure)(ADXRS453_PinLocations_t* locations, SPIDRV_Handle_t handle, const void* buffer, uint8_t length);

/**
 * @brief Type of pointer to procedure that provides means of sending and receining arbitrary buffer to a device with
 * the SPI bus.
 *
 * This method should be block the execution until entire requested block has been sent.
 * @param[in] pointer for locations structure contains pin and port location for device chip sellect port.
 * @param[in] handle for SPI driver.
 * @param[in] buffer Pointer to buffer that contains the data that should be send to the device.
 * @param[in] length Size of the data stored inside the buffer buffer in bytes.
 * @return pair error codes, sending error code and receiving error code see SPI_TransferPairResultCode_t.
 */
typedef SPI_TransferPairResultCode_t (*GyroSPIWriteReadProcedure)(
    ADXRS453_PinLocations_t* locations, SPIDRV_Handle_t handle, void* buffer, uint8_t length);

/**@brief ADXRS453 driver gyroscope interface. */
typedef struct
{
    /**
        * @brief Pointer to function providing means of sending data to ADXRS453 hardware.
        *
        * See the GyroSPIWriteProcedure definition for details regarding usage & implementation requirements.
        */
    GyroSPIWriteProcedure writeProc;
    /**
        * @brief Pointer to function providing means of reading and receiving data from ADXRS453.
        *
        * See the GyroSPIWriteReadProcedure definition for details regarding usage & implementation requirements.
        */
    GyroSPIWriteReadProcedure readProc;
} GyroInterface_t;

/**
 * @brief This type contains ADXRS453 object definitions .
 */
typedef struct ADXRS453_Obj
{
    ADXRS453_PinLocations_t pinLocations;
    GyroInterface_t interface;
} ADXRS453_Obj_t;

/**
 * @brief This type contains transfer return data with error codes  .
 */
typedef struct SPI_TransferReturn
{
    SPI_TransferPairResultCode_t resultCodes;
    /**
     * @brief This union contains returned data for sensor eg. temp, rate, and raw data .
     */
    union {
        int16_t sensorResult;
        uint32_t dataResult;
    } result;
} SPI_TransferReturn_t;

/**
 * @brief This procedure initializes the chip sellect pins for define gyroscope object.
 * *
 * @param[in] gyro Pointer to the ADXRS453 gyroscope object.
 *
 * This procedure does not verify whether the spi driver is successfull initilized.
 */

void ADXRS453Spi_Init(ADXRS453_Obj_t* gyro);

/**
 * @brief This procedure initializes the ADXRS453 driver and sending initilization packets to ADXRS453.
 * @param[in] gyro Pointer to the ADXRS453 gyroscope object.
 * @param[in] handle for SPI driver handle.
 *
 * This procedure does not verify whether the spi driver is successfull initilized.
 */

void ADXRS453_Init(ADXRS453_Obj_t* gyro, SPIDRV_Handle_t handle);
/**
 * @brief This procedure is a implementation of interface GyroSPIWriteProcedure see GyroSPIWriteProcedure.
 *
 * This procedure uses emdrv libraries for communication via SPI.
 */

Ecode_t SPISendB(ADXRS453_PinLocations_t* locations, SPIDRV_Handle_t handle, const void* buffer, uint8_t length);

/**
 * @brief This procedure is part of implementation of interface GyroSPIWriteReadProcedure see  GyroSPIWriteReadProcedure..
 *
 * This procedure uses emdrv libraries for communication via SPI.
 */

Ecode_t SPIRecvB(ADXRS453_PinLocations_t* locations, SPIDRV_Handle_t handle, void* buffer, uint8_t length);
/**
 * @brief This procedure is a implementation of interface GyroSPIWriteReadProcedure see GyroSPIWriteReadProcedure.
 *
 * This procedure uses emdrv libraries for communication via SPI.
 */

SPI_TransferPairResultCode_t SPISendRecvB(ADXRS453_PinLocations_t* locations, SPIDRV_Handle_t handle, void* buffer, uint8_t length);

/**
 * @brief Gets register Value for ADXRS453.
 *
 * @param[in] gyro Object ADXRS453 Pointer.
 * @param[in] handle SPI handler.
 * @param[in] registerAddress register addressr.
 * @return type of transfer return see SPI_TransferReturn_t
 */

SPI_TransferReturn_t ADXRS453_GetRegisterValue(ADXRS453_Obj_t* gyro, SPIDRV_Handle_t handle, uint8_t registerAddress);
/**
 * @brief Sets register Value for ADXRS453.
 *
 * @param[in] gyro Object ADXRS453 Pointer.
 * @param[in] handle SPI handler.
 * @param[in] registerAddress register addressr.
 * @param[in] registerValue register value.
 * @return type of transfer return see SPI_TransferReturn_t
 */
SPI_TransferReturn_t ADXRS453_SetRegisterValue(
    ADXRS453_Obj_t* gyro, SPIDRV_Handle_t handle, uint8_t registerAddress, uint16_t registerValue);

/**
 * @brief gets raw sensor data of ADXRS453.
 *
 * @param[in] gyro Object ADXRS453 Pointer.
 * @param[in] handle SPI handler.
 * @return type of transfer return see SPI_TransferReturn_t
 */

SPI_TransferReturn_t ADXRS453_GetSensorData(ADXRS453_Obj_t* gyro, SPIDRV_Handle_t handle);

/**
 * @brief gets the rate data and converts it to degrees/second..
 *
 * @param[in] gyro Object ADXRS453 Pointer.
 * @param[in] handle SPI handler.
 * @return type of transfer return see SPI_TransferReturn_t
 */

SPI_TransferReturn_t ADXRS453_GetRate(ADXRS453_Obj_t* gyro, SPIDRV_Handle_t handle);

/**
 * @brief gets the temperature sensor data and converts it to degrees Celsius.
 *
 * @param[in] gyro Object ADXRS453 Pointer.
 * @param[in] handle SPI handler.
 * @return type of transfer return see SPI_TransferReturn_t
 */
SPI_TransferReturn_t ADXRS453_GetTemperature(ADXRS453_Obj_t* gyro, SPIDRV_Handle_t handle);

/**
 * @brief generates the command for ADXRS453.
 *
 * @param[in] commandByte command byte  .
 * @param[in] registerAddress register address.
 * @param[in] registerValue register value.
 * @param[out] sendBuffer Pointer to buffer that will contains the command.
 * @return type of transfer return see SPI_TransferReturn_t
 */
void GenerateCommand(uint8_t commandByte, uint8_t registerAddress, uint16_t registerValue, uint8_t* sendBuffer);

/** @}*/
#ifdef __cplusplus
}
#endif
#endif // __ADXRS453_H__
