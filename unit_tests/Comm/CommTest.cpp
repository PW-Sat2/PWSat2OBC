#include <string>
#include <algorithm>
#include <em_i2c.h>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gmock/gmock-matchers.h"
#include "comm/comm.h"
#include "system.h"

using testing::_;
using testing::Eq;
using testing::Ne;
using testing::Ge;
using testing::StrEq;
using testing::Return;
using testing::Invoke;

static const uint8_t ReceiverAddress = 0x60;
static const uint8_t TransmitterAddress = 0x61;

static const uint8_t ReceiverGetFrameCount = 0x21;
static const uint8_t ReceiverRemoveFrame = 0x24;
static const uint8_t ReceiverReset = 0xAA;

static const uint8_t HardwareReset = 0xAB;

static const uint8_t TransmitterSendFrame = 0x10;
static const uint8_t TransmitterClearBeacon = 0x1f;
static const uint8_t TransmitterSetIdleState = 0x24;
static const uint8_t TransmitterSetBitRate = 0x28;
static const uint8_t TransmitterGetState = 0x41;
static const uint8_t TransmitterReset = 0xAA;

struct I2CMock
{
    MOCK_METHOD4(
        I2CWrite, I2C_TransferReturn_TypeDef(uint8_t address, uint8_t command, uint8_t* inData, uint16_t length));

    MOCK_METHOD6(I2CRead,
        I2C_TransferReturn_TypeDef(uint8_t address,
                     uint8_t command,
                     uint8_t* inData,
                     uint16_t inLength,
                     uint8_t* outData,
                     uint16_t outLength));
};

static I2CMock* mockPtr = NULL;

static I2C_TransferReturn_TypeDef TestI2CWrite(uint8_t address, uint8_t* inData, uint16_t length)
{
    if (mockPtr != NULL)
    {
        return mockPtr->I2CWrite(address, (length > 0 && inData != NULL) ? *inData : 0, inData, length);
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
        return mockPtr->I2CRead(
            address, (inLength > 0 && inData != NULL) ? *inData : 0, inData, inLength, outData, outLength);
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
    EXPECT_CALL(i2c, I2CRead(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(i2c, I2CWrite(_, _, _, _)).Times(0);
    CommInitialize(&commObject, &low);
}

TEST_F(CommTest, TestHardwareReset)
{
    EXPECT_CALL(i2c, I2CWrite(ReceiverAddress, HardwareReset, Ne(nullptr), 1)).WillOnce(Return(i2cTransferDone));
    EXPECT_CALL(i2c, I2CWrite(TransmitterAddress, TransmitterReset, Ne(nullptr), 1)).WillOnce(Return(i2cTransferDone));
    EXPECT_CALL(i2c, I2CWrite(ReceiverAddress, ReceiverReset, Ne(nullptr), 1)).WillOnce(Return(i2cTransferDone));
    const auto status = CommReset(&comm);
    ASSERT_THAT(status, Eq(true));
}

TEST_F(CommTest, TestHardwareResetFailureOnHardware)
{
    EXPECT_CALL(i2c, I2CWrite(ReceiverAddress, HardwareReset, _, 1)).WillOnce(Return(i2cTransferNack));
    const auto status = CommReset(&comm);
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestHardwareResetFailureOnReceiver)
{
    EXPECT_CALL(i2c, I2CWrite(ReceiverAddress, HardwareReset, _, 1)).WillOnce(Return(i2cTransferDone));
    EXPECT_CALL(i2c, I2CWrite(TransmitterAddress, TransmitterReset, _, 1)).WillOnce(Return(i2cTransferDone));
    EXPECT_CALL(i2c, I2CWrite(ReceiverAddress, ReceiverReset, _, 1)).WillOnce(Return(i2cTransferNack));
    const auto status = CommReset(&comm);
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestHardwareResetFailureOnTransmitter)
{
    EXPECT_CALL(i2c, I2CWrite(ReceiverAddress, HardwareReset, _, 1)).WillOnce(Return(i2cTransferDone));
    EXPECT_CALL(i2c, I2CWrite(TransmitterAddress, TransmitterReset, _, 1)).WillOnce(Return(i2cTransferNack));
    const auto status = CommReset(&comm);
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestTransmitterReset)
{
    EXPECT_CALL(i2c, I2CWrite(TransmitterAddress, TransmitterReset, Ne(nullptr), 1)).WillOnce(Return(i2cTransferDone));
    const auto status = CommResetTransmitter(&comm);
    ASSERT_THAT(status, Eq(true));
}

TEST_F(CommTest, TestTransmitterResetFailure)
{
    EXPECT_CALL(i2c, I2CWrite(TransmitterAddress, TransmitterReset, _, 1)).WillOnce(Return(i2cTransferNack));
    const auto status = CommResetTransmitter(&comm);
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestReceiverReset)
{
    EXPECT_CALL(i2c, I2CWrite(ReceiverAddress, ReceiverReset, Ne(nullptr), 1)).WillOnce(Return(i2cTransferDone));
    const auto status = CommResetReceiver(&comm);
    ASSERT_THAT(status, Eq(true));
}

TEST_F(CommTest, TestReceiverResetFailure)
{
    EXPECT_CALL(i2c, I2CWrite(ReceiverAddress, ReceiverReset, _, 1)).WillOnce(Return(i2cTransferNack));
    const auto status = CommResetReceiver(&comm);
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestFrameRemoval)
{
    EXPECT_CALL(i2c, I2CWrite(ReceiverAddress, ReceiverRemoveFrame, Ne(nullptr), 1)).WillOnce(Return(i2cTransferDone));
    const auto status = CommRemoveFrame(&comm);
    ASSERT_THAT(status, Eq(true));
}

TEST_F(CommTest, TestFrameRemovalFailure)
{
    EXPECT_CALL(i2c, I2CWrite(ReceiverAddress, ReceiverRemoveFrame, _, 1)).WillOnce(Return(i2cTransferNack));
    const auto status = CommRemoveFrame(&comm);
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestGetFrameCountFailure)
{
    EXPECT_CALL(i2c, I2CRead(ReceiverAddress, ReceiverGetFrameCount, _, _, _, _)).WillOnce(Return(i2cTransferNack));
    const auto result = CommGetFrameCount(&comm);
    ASSERT_THAT(result.status, Eq(false));
    ASSERT_THAT(result.frameCount, Eq(0));
}

TEST_F(CommTest, TestGetFrameCount)
{
    EXPECT_CALL(i2c, I2CRead(ReceiverAddress, ReceiverGetFrameCount, Ne(nullptr), 1, Ne(nullptr), 1))
        .WillOnce(Invoke([](uint8_t /*address*/,
            uint8_t /*command*/,
            uint8_t* /*inData*/,
            uint16_t /*inLength*/,
            uint8_t* outData,
            uint16_t /*outLength*/) {
            *outData = 31;
            return i2cTransferDone;
        }));

    const auto result = CommGetFrameCount(&comm);
    ASSERT_THAT(result.status, Eq(true));
    ASSERT_THAT(result.frameCount, Eq(31));
}

TEST_F(CommTest, TestClearBeaconFailure)
{
    EXPECT_CALL(i2c, I2CWrite(TransmitterAddress, TransmitterClearBeacon, _, 1)).WillOnce(Return(i2cTransferNack));
    const auto status = CommClearBeacon(&comm);
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestClearBeacon)
{
    EXPECT_CALL(i2c, I2CWrite(TransmitterAddress, TransmitterClearBeacon, Ne(nullptr), 1))
        .WillOnce(Return(i2cTransferDone));
    const auto status = CommClearBeacon(&comm);
    ASSERT_THAT(status, Eq(true));
}

TEST_F(CommTest, TestSetIdleStateFailure)
{
    EXPECT_CALL(i2c, I2CWrite(TransmitterAddress, TransmitterSetIdleState, _, 2)).WillOnce(Return(i2cTransferNack));
    const auto status = CommSetTransmitterStateWhenIdle(&comm, CommTransmitterOn);
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestSetIdleState)
{
    EXPECT_CALL(i2c, I2CWrite(TransmitterAddress, TransmitterSetIdleState, Ne(nullptr), 2))
        .WillOnce(Return(i2cTransferDone));
    const auto status = CommSetTransmitterStateWhenIdle(&comm, CommTransmitterOn);
    ASSERT_THAT(status, Eq(true));
}

TEST_F(CommTest, TestSetIdleStateCommandOn)
{
    EXPECT_CALL(i2c, I2CWrite(TransmitterAddress, TransmitterSetIdleState, Ne(nullptr), 2))
        .WillOnce(Invoke([](uint8_t /*address*/, uint8_t /*command*/, uint8_t* inData, uint16_t /*length*/) {
            EXPECT_THAT(inData[1], Eq(1));
            return i2cTransferDone;
        }));
    const auto status = CommSetTransmitterStateWhenIdle(&comm, CommTransmitterOn);
    ASSERT_THAT(status, Eq(true));
}

TEST_F(CommTest, TestSetIdleStateCommandOff)
{
    EXPECT_CALL(i2c, I2CWrite(TransmitterAddress, TransmitterSetIdleState, Ne(nullptr), 2))
        .WillOnce(Invoke([](uint8_t /*address*/, uint8_t /*command*/, uint8_t* inData, uint16_t /*length*/) {
            EXPECT_THAT(inData[1], Eq(0));
            return i2cTransferDone;
        }));
    const auto status = CommSetTransmitterStateWhenIdle(&comm, CommTransmitterOff);
    ASSERT_THAT(status, Eq(true));
}

TEST_F(CommTest, TestSetBitRateFailure)
{
    EXPECT_CALL(i2c, I2CWrite(TransmitterAddress, TransmitterSetBitRate, Ne(nullptr), 2))
        .WillOnce(Return(i2cTransferNack));
    const auto status = CommSetTransmitterBitRate(&comm, Comm1200bps);
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestSetBitRate)
{
    EXPECT_CALL(i2c, I2CWrite(TransmitterAddress, TransmitterSetBitRate, Ne(nullptr), 2))
        .WillOnce(Return(i2cTransferDone));
    const auto status = CommSetTransmitterBitRate(&comm, Comm1200bps);
    ASSERT_THAT(status, Eq(true));
}

TEST_F(CommTest, TestSetBitRateCommand)
{
    EXPECT_CALL(i2c, I2CWrite(TransmitterAddress, TransmitterSetBitRate, Ne(nullptr), 2))
        .WillOnce(Invoke([](uint8_t /*address*/, uint8_t /*command*/, uint8_t* inData, uint16_t /*length*/) {
            EXPECT_THAT(inData[1], Eq(1));
            return i2cTransferDone;
        }))
        .WillOnce(Invoke([](uint8_t /*address*/, uint8_t /*command*/, uint8_t* inData, uint16_t /*length*/) {
            EXPECT_THAT(inData[1], Eq(2));
            return i2cTransferDone;
        }))
        .WillOnce(Invoke([](uint8_t /*address*/, uint8_t /*command*/, uint8_t* inData, uint16_t /*length*/) {
            EXPECT_THAT(inData[1], Eq(4));
            return i2cTransferDone;
        }))
        .WillOnce(Invoke([](uint8_t /*address*/, uint8_t /*command*/, uint8_t* inData, uint16_t /*length*/) {
            EXPECT_THAT(inData[1], Eq(8));
            return i2cTransferDone;
        }));
    auto status = CommSetTransmitterBitRate(&comm, Comm1200bps);
    ASSERT_THAT(status, Eq(true));
    status = CommSetTransmitterBitRate(&comm, Comm2400bps);
    ASSERT_THAT(status, Eq(true));
    status = CommSetTransmitterBitRate(&comm, Comm4800bps);
    ASSERT_THAT(status, Eq(true));
    status = CommSetTransmitterBitRate(&comm, Comm9600bps);
    ASSERT_THAT(status, Eq(true));
}

TEST_F(CommTest, TestGetTransmitterStateFailure)
{
    CommTransmitterState state;
    EXPECT_CALL(i2c, I2CRead(TransmitterAddress, TransmitterGetState, Ne(nullptr), 1, _, _))
        .WillOnce(Return(i2cTransferNack));
    const auto status = CommGetTransmitterState(&comm, &state);
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestGetTransmitterInvalidResponse)
{
    CommTransmitterState state;
    EXPECT_CALL(i2c, I2CRead(TransmitterAddress, TransmitterGetState, Ne(nullptr), 1, Ne(nullptr), 1))
        .WillOnce(Invoke([](uint8_t /*address*/,
            uint8_t /*command*/,
            uint8_t* /*inData*/,
            uint16_t /*inLength*/,
            uint8_t* outData,
            uint16_t /*outLength*/) {
            *outData = 0xff;
            return i2cTransferDone;
        }));
    const auto status = CommGetTransmitterState(&comm, &state);
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestGetTransmitterResponse)
{
    CommTransmitterState state;
    EXPECT_CALL(i2c, I2CRead(TransmitterAddress, TransmitterGetState, Ne(nullptr), 1, Ne(nullptr), 1))
        .WillOnce(Invoke([](uint8_t /*address*/,
            uint8_t /*command*/,
            uint8_t* /*inData*/,
            uint16_t /*inLength*/,
            uint8_t* outData,
            uint16_t /*outLength*/) {
            *outData = 0x7f;
            return i2cTransferDone;
        }));
    const auto status = CommGetTransmitterState(&comm, &state);
    ASSERT_THAT(status, Eq(true));
    ASSERT_THAT(state.BeaconState, Eq(true));
    ASSERT_THAT(state.StateWhenIdle, Eq(CommTransmitterOn));
    ASSERT_THAT(state.TransmitterBitRate, Eq(Comm9600bps));
}

TEST_F(CommTest, TestGetBaseLineTransmitterResponse)
{
    CommTransmitterState state;
    EXPECT_CALL(i2c, I2CRead(TransmitterAddress, TransmitterGetState, Ne(nullptr), 1, Ne(nullptr), 1))
        .WillOnce(Invoke([](uint8_t /*address*/,
            uint8_t /*command*/,
            uint8_t* /*inData*/,
            uint16_t /*inLength*/,
            uint8_t* outData,
            uint16_t /*outLength*/) {
            *outData = 0x0;
            return i2cTransferDone;
        }));
    const auto status = CommGetTransmitterState(&comm, &state);
    ASSERT_THAT(status, Eq(true));
    ASSERT_THAT(state.BeaconState, Eq(false));
    ASSERT_THAT(state.StateWhenIdle, Eq(CommTransmitterOff));
    ASSERT_THAT(state.TransmitterBitRate, Eq(Comm1200bps));
}

TEST_F(CommTest, TestGetMixedLineTransmitterResponse)
{
    CommTransmitterState state;
    EXPECT_CALL(i2c, I2CRead(TransmitterAddress, TransmitterGetState, Ne(nullptr), 1, Ne(nullptr), 1))
        .WillOnce(Invoke([](uint8_t /*address*/,
            uint8_t /*command*/,
            uint8_t* /*inData*/,
            uint16_t /*inLength*/,
            uint8_t* outData,
            uint16_t /*outLength*/) {
            *outData = 0x0a;
            return i2cTransferDone;
        }));
    const auto status = CommGetTransmitterState(&comm, &state);
    ASSERT_THAT(status, Eq(true));
    ASSERT_THAT(state.BeaconState, Eq(true));
    ASSERT_THAT(state.StateWhenIdle, Eq(CommTransmitterOff));
    ASSERT_THAT(state.TransmitterBitRate, Eq(Comm4800bps));
}

TEST_F(CommTest, TestSendTooLongFrame)
{
    uint8_t buffer[10] = {0};
    const auto status = CommSendFrame(&comm, buffer, COMM_MAX_FRAME_CONTENTS_SIZE + 1);
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestSendFrameFailure)
{
    uint8_t buffer[10] = {0};
    EXPECT_CALL(i2c, I2CRead(TransmitterAddress, TransmitterSendFrame, Ne(nullptr), COUNT_OF(buffer) + 1, _, _))
        .WillOnce(Return(i2cTransferNack));
    const auto status = CommSendFrame(&comm, buffer, COUNT_OF(buffer));
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestSendFrame)
{
    uint8_t buffer[] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc};
    EXPECT_CALL(
        i2c, I2CRead(TransmitterAddress, TransmitterSendFrame, Ne(nullptr), COUNT_OF(buffer) + 1, Ne(nullptr), 1))
        .WillOnce(Invoke([&](uint8_t /*address*/,
            uint8_t /*command*/,
            uint8_t* inData,
            uint16_t length,
            uint8_t* outData,
            uint16_t /*outLength*/) {
            const uint8_t expected[] = {
                TransmitterSendFrame, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc};
            EXPECT_THAT(std::equal(inData, inData + length, std::begin(expected), std::end(expected)), Eq(true));
            *outData = 0;
            return i2cTransferDone;
        }));
    const auto status = CommSendFrame(&comm, buffer, COUNT_OF(buffer));
    ASSERT_THAT(status, Eq(true));
}

TEST_F(CommTest, TestSendFrameRejectedByHardware)
{
    uint8_t buffer[] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc};
    EXPECT_CALL(i2c, I2CRead(TransmitterAddress, TransmitterSendFrame, _, _, Ne(nullptr), 1))
        .WillOnce(Invoke([&](uint8_t /*address*/,
            uint8_t /*command*/,
            uint8_t* /*inData*/,
            uint16_t /*length*/,
            uint8_t* outData,
            uint16_t /*outLength*/) {
            *outData = 0xff;
            return i2cTransferDone;
        }));
    const auto status = CommSendFrame(&comm, buffer, COUNT_OF(buffer));
    ASSERT_THAT(status, Eq(false));
}
