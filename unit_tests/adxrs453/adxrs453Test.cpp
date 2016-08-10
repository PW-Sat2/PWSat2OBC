#include <string>
#include <algorithm>
#include <em_i2c.h>
#include <tuple>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gmock/gmock-matchers.h"
#include "OsMock.hpp"
#include "adxrs453/adxrs453.h"
#include "os/os.hpp"
#include "system.h"
#include "spidrv.h"

using testing::_;
using testing::Eq;
using testing::Ne;
using testing::Ge;
using testing::StrEq;
using testing::Return;
using testing::Invoke;


struct SPIMock
{
    MOCK_METHOD4(
        SPIWrite, Ecode_t(ADXRS453_PinLocations_t *locations,
        		SPIDRV_Handle_t handle, const void * buffer,
				uint8_t length));

    MOCK_METHOD4(SPIWriteRead,
    		SPI_TransferPairResultCode_t(ADXRS453_PinLocations_t *locations,
    				SPIDRV_Handle_t handle,
					const void * buffer,
					uint8_t length));
};

static SPIMock* mockPtr = NULL;

static Ecode_t TestSPIWrite(ADXRS453_PinLocations_t *locations,
		SPIDRV_Handle_t handle, const void * buffer,
		uint8_t length)
{
    if (mockPtr != NULL)
    {
        return mockPtr->SPIWrite(locations, handle, buffer, length);
    }
    else
    {
        return ECODE_EMDRV_SPIDRV_ILLEGAL_HANDLE;
    }
}

static SPI_TransferPairResultCode_t TestSPIWriteRead(ADXRS453_PinLocations_t *locations,
		SPIDRV_Handle_t handle,
		const void * buffer,
		uint8_t length)
{
    if (mockPtr != NULL)
    {
        return mockPtr->SPIWriteRead(
            locations, handle, buffer, length);
    }
    else
    {
    	SPI_TransferPairResultCode_t returncodes;
    	returncodes.resultCodeWrite=ECODE_EMDRV_SPIDRV_ILLEGAL_HANDLE;
    	returncodes.resultCodeRead =ECODE_EMDRV_SPIDRV_ILLEGAL_HANDLE;
        return returncodes;
    }
}

static void SetupAdxrs453(ADXRS453_Obj_t *gyro)
{
	SPIDRV_HandleData_t handleData;
	SPIDRV_Handle_t handle = &handleData;
	GyroInterface_t interface;
	interface.writeProc=TestSPIWrite;
	interface.readProc=TestSPIWriteRead;
	gyro->interface=interface;
	ADXRS453_Init(gyro,handle);

}

class adxrs453Test : public testing::Test
{
  public:
	adxrs453Test();
    ~adxrs453Test();

  protected:
    ADXRS453_Obj_t gyro;
    testing::NiceMock<OSMock> system;
    SPIMock spimock;
};

adxrs453Test::adxrs453Test()
{
    SetupAdxrs453(&gyro);
    mockPtr = &spimock;
}

adxrs453Test::~adxrs453Test()
{
    mockPtr = nullptr;
}

TEST_F(adxrs453Test, TestInitializationWithInitialisationProcedure)
{
	SPIDRV_HandleData_t handleData;
		SPIDRV_Handle_t handle = &handleData;
	GyroInterface_t interface;
	interface.writeProc=TestSPIWrite;
	interface.readProc=TestSPIWriteRead;
	gyro.interface=interface;
	EXPECT_CALL(spimock, SPIWrite(_, _, _, _)).Times(5);
	ADXRS453_Init(&gyro,handle);
}
TEST_F(adxrs453Test, TestRunFunctionInterfaceWhenGetSensorValue)
{
	SPIDRV_HandleData_t handleData;
		SPIDRV_Handle_t handle = &handleData;
	GyroInterface_t interface;
	interface.writeProc=TestSPIWrite;
	interface.readProc=TestSPIWriteRead;
	gyro.interface=interface;
	EXPECT_CALL(spimock, SPIWriteRead(_, _, _, _)).Times(2);
	ADXRS453_GetTemperature(&gyro,handle);
	ADXRS453_GetRate(&gyro,handle);
}

TEST_F(adxrs453Test, TestWriteBusyErrorSPICommunication)
{
	SPIDRV_HandleData_t handleData;
	SPIDRV_Handle_t handle = &handleData;
	GyroInterface_t interface;
	interface.writeProc=TestSPIWrite;
	interface.readProc=TestSPIWriteRead;
	gyro.interface=interface;
	SPI_TransferReturn result;
	EXPECT_CALL(spimock, SPIWrite(_, _, _, _)).WillOnce(Return(ECODE_EMDRV_SPIDRV_BUSY));
	ON_CALL(spimock, SPIWrite(_, _, _, _)).WillByDefault(Return(ECODE_EMDRV_SPIDRV_BUSY));
	result = ADXRS453_SetRegisterValue(&gyro,handle,0,0);
	EXPECT_EQ(result.resultCodes.resultCodeWrite, ECODE_EMDRV_SPIDRV_BUSY);
}

TEST_F(adxrs453Test, TestReadBusyErrorSPICommunication)
{
	SPIDRV_HandleData_t handleData;
	SPIDRV_Handle_t handle = &handleData;
	GyroInterface_t interface;
	interface.writeProc=TestSPIWrite;
	interface.readProc=TestSPIWriteRead;
	gyro.interface=interface;
	SPI_TransferReturn result;
	SPI_TransferPairResultCode failresult;
	failresult.resultCodeWrite=ECODE_EMDRV_SPIDRV_BUSY;
	failresult.resultCodeRead=ECODE_EMDRV_SPIDRV_BUSY;
	EXPECT_CALL(spimock, SPIWriteRead(_, _, _, _)).WillOnce(Return(failresult));
	ON_CALL(spimock, SPIWriteRead(_, _, _, _)).WillByDefault(Return(failresult));
	result = ADXRS453_GetRegisterValue(&gyro,handle,0);
	EXPECT_EQ(result.resultCodes.resultCodeWrite, ECODE_EMDRV_SPIDRV_BUSY);
	EXPECT_EQ(result.resultCodes.resultCodeRead, ECODE_EMDRV_SPIDRV_BUSY);
}

TEST_F(adxrs453Test, TestReadBusyErrorTempSPICommunication)
{
	SPIDRV_HandleData_t handleData;
	SPIDRV_Handle_t handle = &handleData;
	GyroInterface_t interface;
	interface.writeProc=TestSPIWrite;
	interface.readProc=TestSPIWriteRead;
	gyro.interface=interface;
	SPI_TransferReturn result;
	SPI_TransferPairResultCode failresult;
	failresult.resultCodeWrite=ECODE_EMDRV_SPIDRV_BUSY;
	failresult.resultCodeRead=ECODE_EMDRV_SPIDRV_BUSY;
	EXPECT_CALL(spimock, SPIWriteRead(_, _, _, _)).WillOnce(Return(failresult));
	ON_CALL(spimock, SPIWriteRead(_, _, _, _)).WillByDefault(Return(failresult));
	result = ADXRS453_GetTemperature(&gyro,handle);
	EXPECT_EQ(result.resultCodes.resultCodeWrite, ECODE_EMDRV_SPIDRV_BUSY);
	EXPECT_EQ(result.resultCodes.resultCodeRead, ECODE_EMDRV_SPIDRV_BUSY);

}

TEST_F(adxrs453Test, TestReadBusyErrorRateSPICommunication)
{
	SPIDRV_HandleData_t handleData;
	SPIDRV_Handle_t handle = &handleData;
	GyroInterface_t interface;
	interface.writeProc=TestSPIWrite;
	interface.readProc=TestSPIWriteRead;
	gyro.interface=interface;
	SPI_TransferReturn result;
	SPI_TransferPairResultCode failresult;
	failresult.resultCodeWrite=ECODE_EMDRV_SPIDRV_BUSY;
	failresult.resultCodeRead=ECODE_EMDRV_SPIDRV_BUSY;
	EXPECT_CALL(spimock, SPIWriteRead(_, _, _, _)).WillOnce(Return(failresult));
	ON_CALL(spimock, SPIWriteRead(_, _, _, _)).WillByDefault(Return(failresult));
	result = ADXRS453_GetRate(&gyro,handle);
	EXPECT_EQ(result.resultCodes.resultCodeWrite, ECODE_EMDRV_SPIDRV_BUSY);
	EXPECT_EQ(result.resultCodes.resultCodeRead, ECODE_EMDRV_SPIDRV_BUSY);
}

