#include <string>
#include <em_i2c.h>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gmock/gmock-matchers.h"
#include "comm/comm.h"
#include "system.h"

using testing::_;
using testing::Eq;
using testing::Ne;
using testing::StrEq;

struct I2CMock
{
    MOCK_METHOD3(I2CWrite, I2C_TransferReturn_TypeDef(uint8_t address, uint8_t* inData, uint16_t length));
    MOCK_METHOD5(I2CRead,
        I2C_TransferReturn_TypeDef(
                     uint8_t address, uint8_t* inData, uint16_t inLength, uint8_t* outData, uint16_t outLength));
};

static I2CMock* mockPtr = NULL;

static I2C_TransferReturn_TypeDef TestI2CWrite(uint8_t address, uint8_t* inData, uint16_t length)
{
    if (mockPtr != NULL)
    {
        return mockPtr->I2CWrite(address, inData, length);
    }
    else
    {
        return i2cTransferNack;
    }
}

static I2C_TransferReturn_TypeDef TestI2CRead(
    uint8_t address, uint8_t* inData, uint16_t inLength, uint8_t* outData, uint16_t outLength)
{
    if (mockPtr != NULL)
    {
        return mockPtr->I2CRead(address, inData, inLength, outData, outLength);
    }
    else
    {
        return i2cTransferNack;
    }
}

class CommTest : public testing::Test
{
  public:
    CommTest();
    ~CommTest();

  protected:
    CommObject comm;
    I2CMock i2c;
};

CommTest::CommTest()
{
    CommLowInterface low;
    low.writeProc = TestI2CWrite;
    low.readProc = TestI2CRead;
    CommInitialize(&comm, &low);
    mockPtr = &i2c;
}

CommTest::~CommTest()
{
    mockPtr = nullptr;
}

TEST_F(CommTest, ParseReceivedFrame)
{
    uint8_t raw[] = {3, 0, 44, 1, 64, 1, 65, 66, 67, 0};
    Frame* frame = (Frame*)raw;

    ASSERT_THAT(frame->Size, Eq(3));
    ASSERT_THAT(frame->Doppler, Eq(300));
    ASSERT_THAT(frame->RSSI, Eq(320));
    ASSERT_THAT((char*)frame->Contents, StrEq("ABC"));
}

TEST_F(CommTest, TestInitializationDoesNotTouchHardware)
{
    CommObject commObject;
    CommLowInterface low;
    low.writeProc = TestI2CWrite;
    low.readProc = TestI2CRead;
    EXPECT_CALL(i2c, I2CRead(_, _, _, _, _)).Times(0);
    EXPECT_CALL(i2c, I2CWrite(_, _, _)).Times(0);
    CommInitialize(&commObject, &low);
}
