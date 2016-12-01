#include <algorithm>
#include <em_i2c.h>
#include <gsl/span>
#include <string>
#include <tuple>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gmock/gmock-matchers.h"
#include "OsMock.hpp"
#include "comm/comm.h"
#include "i2c/I2CMock.hpp"
#include "i2c/i2c.h"
#include "os/os.hpp"
#include "system.h"

using testing::_;
using testing::Eq;
using testing::Ne;
using testing::Ge;
using testing::StrEq;
using testing::Return;
using testing::Invoke;
using gsl::span;

using namespace devices::comm;

static const uint8_t ReceiverAddress = 0x60;
static const uint8_t TransmitterAddress = 0x62;

static const uint8_t ReceverGetTelemetry = 0x1A;
static const uint8_t ReceiverGetFrameCount = 0x21;
static const uint8_t ReceiverGetFrame = 0x22;
static const uint8_t ReceiverRemoveFrame = 0x24;
static const uint8_t ReceiverReset = 0xAA;

static const uint8_t HardwareReset = 0xAB;

static const uint8_t TransmitterSendFrame = 0x10;
static const uint8_t TransmitterSetBeacon = 0x14;
static const uint8_t TransmitterClearBeacon = 0x1f;
static const uint8_t TransmitterSetIdleState = 0x24;
static const uint8_t TransmitterGetTelemetry = 0x25;
static const uint8_t TransmitterSetBitRate = 0x28;
static const uint8_t TransmitterGetState = 0x41;
static const uint8_t TransmitterReset = 0xAA;

static OSReset SetupComm(CommObject& comm, OSMock& system)
{
    auto reset = InstallProxy(&system);
    ON_CALL(system, CreateEventGroup()).WillByDefault(Return(reinterpret_cast<OSEventGroupHandle>(&comm)));

    EXPECT_THAT(comm.Initialize(), Eq(OSResult::Success));

    return reset;
}

struct FrameHandlerMock : IHandleFrame
{
    MOCK_METHOD2(HandleFrame, void(CommObject&, CommFrame&));
};

class CommTest : public testing::Test
{
  public:
    CommTest();

  protected:
    CommObject comm;
    FrameHandlerMock frameHandler;
    testing::NiceMock<OSMock> system;
    I2CBusMock i2c;
    OSReset reset;
    I2CBus low;
};

CommTest::CommTest() : comm(i2c, frameHandler)
{
    reset = SetupComm(comm, system);
}

TEST_F(CommTest, TestInitializationDoesNotTouchHardware)
{
    CommObject commObject(i2c, frameHandler);

    EXPECT_CALL(i2c, I2CWriteRead(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(i2c, I2CWrite(_, _, _, _)).Times(0);

    commObject.Initialize();
}

TEST_F(CommTest, TestInitializationAllocationFailure)
{
    CommObject commObject(i2c, frameHandler);

    EXPECT_CALL(system, CreateEventGroup()).WillOnce(Return(nullptr));

    const auto status = commObject.Initialize();

    ASSERT_THAT(status, Ne(OSResult::Success));
}

TEST_F(CommTest, TestInitialization)
{
    CommObject commObject(i2c, frameHandler);

    const auto status = commObject.Initialize();

    ASSERT_THAT(status, Eq(OSResult::Success));
}

TEST_F(CommTest, TestHardwareReset)
{
    EXPECT_CALL(i2c, I2CWrite(ReceiverAddress, HardwareReset, Ne(nullptr), 1)).WillOnce(Return(I2CResultOK));
    EXPECT_CALL(i2c, I2CWrite(TransmitterAddress, TransmitterReset, Ne(nullptr), 1)).WillOnce(Return(I2CResultOK));
    EXPECT_CALL(i2c, I2CWrite(ReceiverAddress, ReceiverReset, Ne(nullptr), 1)).WillOnce(Return(I2CResultOK));
    const auto status = comm.Reset();
    ASSERT_THAT(status, Eq(true));
}

TEST_F(CommTest, TestHardwareResetFailureOnHardware)
{
    EXPECT_CALL(i2c, I2CWrite(ReceiverAddress, HardwareReset, _, 1)).WillOnce(Return(I2CResultNack));
    const auto status = comm.Reset();
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestHardwareResetFailureOnReceiver)
{
    EXPECT_CALL(i2c, I2CWrite(ReceiverAddress, HardwareReset, Ne(nullptr), 1)).WillOnce(Return(I2CResultOK));
    EXPECT_CALL(i2c, I2CWrite(ReceiverAddress, ReceiverReset, Ne(nullptr), 1)).WillOnce(Return(I2CResultNack));
    ON_CALL(i2c, I2CWrite(TransmitterAddress, _, _, _)).WillByDefault(Return(I2CResultOK));
    const auto status = comm.Reset();
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestHardwareResetFailureOnTransmitter)
{
    EXPECT_CALL(i2c, I2CWrite(TransmitterAddress, TransmitterReset, _, 1)).WillOnce(Return(I2CResultNack));
    EXPECT_CALL(i2c, I2CWrite(ReceiverAddress, _, _, _)).WillRepeatedly(Return(I2CResultOK));
    const auto status = comm.Reset();
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestTransmitterReset)
{
    EXPECT_CALL(i2c, I2CWrite(TransmitterAddress, TransmitterReset, Ne(nullptr), 1)).WillOnce(Return(I2CResultOK));
    const auto status = comm.ResetTransmitter();
    ASSERT_THAT(status, Eq(true));
}

TEST_F(CommTest, TestTransmitterResetFailure)
{
    EXPECT_CALL(i2c, I2CWrite(TransmitterAddress, TransmitterReset, _, 1)).WillOnce(Return(I2CResultNack));
    const auto status = comm.ResetTransmitter();
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestReceiverReset)
{
    EXPECT_CALL(i2c, I2CWrite(ReceiverAddress, ReceiverReset, Ne(nullptr), 1)).WillOnce(Return(I2CResultOK));
    const auto status = comm.ResetReceiver();
    ASSERT_THAT(status, Eq(true));
}

TEST_F(CommTest, TestReceiverResetFailure)
{
    EXPECT_CALL(i2c, I2CWrite(ReceiverAddress, ReceiverReset, _, 1)).WillOnce(Return(I2CResultNack));
    const auto status = comm.ResetReceiver();
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestFrameRemoval)
{
    EXPECT_CALL(i2c, I2CWrite(ReceiverAddress, ReceiverRemoveFrame, Ne(nullptr), 1)).WillOnce(Return(I2CResultOK));
    const auto status = comm.RemoveFrame();
    ASSERT_THAT(status, Eq(true));
}

TEST_F(CommTest, TestFrameRemovalFailure)
{
    EXPECT_CALL(i2c, I2CWrite(ReceiverAddress, ReceiverRemoveFrame, _, 1)).WillOnce(Return(I2CResultNack));
    const auto status = comm.RemoveFrame();
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestGetFrameCountFailure)
{
    EXPECT_CALL(i2c, I2CWriteRead(ReceiverAddress, ReceiverGetFrameCount, _, _, _, _)).WillOnce(Return(I2CResultNack));
    const auto result = comm.GetFrameCount();
    ASSERT_THAT(result.status, Eq(false));
    ASSERT_THAT(result.frameCount, Eq(0));
}

TEST_F(CommTest, TestGetFrameCount)
{
    EXPECT_CALL(i2c, I2CWriteRead(ReceiverAddress, ReceiverGetFrameCount, Ne(nullptr), 1, Ne(nullptr), 1))
        .WillOnce(Invoke([](uint8_t /*address*/,
            uint8_t /*command*/,
            const uint8_t* /*inData*/,
            uint16_t /*inLength*/,
            uint8_t* outData,
            uint16_t /*outLength*/) {
            *outData = 31;
            return I2CResultOK;
        }));

    const auto result = comm.GetFrameCount();
    ASSERT_THAT(result.status, Eq(true));
    ASSERT_THAT(result.frameCount, Eq(31));
}

TEST_F(CommTest, TestClearBeaconFailure)
{
    EXPECT_CALL(i2c, I2CWrite(TransmitterAddress, TransmitterClearBeacon, _, 1)).WillOnce(Return(I2CResultNack));
    const auto status = comm.ClearBeacon();
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestClearBeacon)
{
    EXPECT_CALL(i2c, I2CWrite(TransmitterAddress, TransmitterClearBeacon, Ne(nullptr), 1)).WillOnce(Return(I2CResultOK));
    const auto status = comm.ClearBeacon();
    ASSERT_THAT(status, Eq(true));
}

TEST_F(CommTest, TestSetIdleStateFailure)
{
    EXPECT_CALL(i2c, I2CWrite(TransmitterAddress, TransmitterSetIdleState, _, 2)).WillOnce(Return(I2CResultNack));
    const auto status = comm.SetTransmitterStateWhenIdle(CommTransmitterOn);
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestSetIdleState)
{
    EXPECT_CALL(i2c, I2CWrite(TransmitterAddress, TransmitterSetIdleState, Ne(nullptr), 2)).WillOnce(Return(I2CResultOK));
    const auto status = comm.SetTransmitterStateWhenIdle(CommTransmitterOn);
    ASSERT_THAT(status, Eq(true));
}

TEST_F(CommTest, TestSetIdleStateCommandOn)
{
    EXPECT_CALL(i2c, I2CWrite(TransmitterAddress, TransmitterSetIdleState, Ne(nullptr), 2))
        .WillOnce(Invoke([](uint8_t /*address*/, uint8_t /*command*/, const uint8_t* inData, uint16_t /*length*/) {
            EXPECT_THAT(inData[1], Eq(1));
            return I2CResultOK;
        }));
    const auto status = comm.SetTransmitterStateWhenIdle(CommTransmitterOn);
    ASSERT_THAT(status, Eq(true));
}

TEST_F(CommTest, TestSetIdleStateCommandOff)
{
    EXPECT_CALL(i2c, I2CWrite(TransmitterAddress, TransmitterSetIdleState, Ne(nullptr), 2))
        .WillOnce(Invoke([](uint8_t /*address*/, uint8_t /*command*/, const uint8_t* inData, uint16_t /*length*/) {
            EXPECT_THAT(inData[1], Eq(0));
            return I2CResultOK;
        }));
    const auto status = comm.SetTransmitterStateWhenIdle(CommTransmitterOff);
    ASSERT_THAT(status, Eq(true));
}

TEST_F(CommTest, TestSetBitRateFailure)
{
    EXPECT_CALL(i2c, I2CWrite(TransmitterAddress, TransmitterSetBitRate, Ne(nullptr), 2)).WillOnce(Return(I2CResultNack));
    const auto status = comm.SetTransmitterBitRate(Comm1200bps);
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestSetBitRate)
{
    EXPECT_CALL(i2c, I2CWrite(TransmitterAddress, TransmitterSetBitRate, Ne(nullptr), 2)).WillOnce(Return(I2CResultOK));
    const auto status = comm.SetTransmitterBitRate(Comm1200bps);
    ASSERT_THAT(status, Eq(true));
}

TEST_F(CommTest, TestSetBitRateCommand)
{
    EXPECT_CALL(i2c, I2CWrite(TransmitterAddress, TransmitterSetBitRate, Ne(nullptr), 2))
        .WillOnce(Invoke([](uint8_t /*address*/, uint8_t /*command*/, const uint8_t* inData, uint16_t /*length*/) {
            EXPECT_THAT(inData[1], Eq(1));
            return I2CResultOK;
        }))
        .WillOnce(Invoke([](uint8_t /*address*/, uint8_t /*command*/, const uint8_t* inData, uint16_t /*length*/) {
            EXPECT_THAT(inData[1], Eq(2));
            return I2CResultOK;
        }))
        .WillOnce(Invoke([](uint8_t /*address*/, uint8_t /*command*/, const uint8_t* inData, uint16_t /*length*/) {
            EXPECT_THAT(inData[1], Eq(4));
            return I2CResultOK;
        }))
        .WillOnce(Invoke([](uint8_t /*address*/, uint8_t /*command*/, const uint8_t* inData, uint16_t /*length*/) {
            EXPECT_THAT(inData[1], Eq(8));
            return I2CResultOK;
        }));
    auto status = comm.SetTransmitterBitRate(Comm1200bps);
    ASSERT_THAT(status, Eq(true));
    status = comm.SetTransmitterBitRate(Comm2400bps);
    ASSERT_THAT(status, Eq(true));
    status = comm.SetTransmitterBitRate(Comm4800bps);
    ASSERT_THAT(status, Eq(true));
    status = comm.SetTransmitterBitRate(Comm9600bps);
    ASSERT_THAT(status, Eq(true));
}

TEST_F(CommTest, TestGetTransmitterStateFailure)
{
    CommTransmitterState state;
    EXPECT_CALL(i2c, I2CWriteRead(TransmitterAddress, TransmitterGetState, Ne(nullptr), 1, _, _)).WillOnce(Return(I2CResultNack));
    const auto status = comm.GetTransmitterState(state);
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestGetTransmitterInvalidResponse)
{
    CommTransmitterState state;
    EXPECT_CALL(i2c, I2CWriteRead(TransmitterAddress, TransmitterGetState, Ne(nullptr), 1, Ne(nullptr), 1))
        .WillOnce(Invoke([](uint8_t /*address*/,
            uint8_t /*command*/,
            const uint8_t* /*inData*/,
            uint16_t /*inLength*/,
            uint8_t* outData,
            uint16_t /*outLength*/) {
            *outData = 0xff;
            return I2CResultOK;
        }));
    const auto status = comm.GetTransmitterState(state);
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestGetTransmitterResponse)
{
    CommTransmitterState state;
    EXPECT_CALL(i2c, I2CWriteRead(TransmitterAddress, TransmitterGetState, Ne(nullptr), 1, Ne(nullptr), 1))
        .WillOnce(Invoke([](uint8_t /*address*/,
            uint8_t /*command*/,
            const uint8_t* /*inData*/,
            uint16_t /*inLength*/,
            uint8_t* outData,
            uint16_t /*outLength*/) {
            *outData = 0x7f;
            return I2CResultOK;
        }));
    const auto status = comm.GetTransmitterState(state);
    ASSERT_THAT(status, Eq(true));
    ASSERT_THAT(state.BeaconState, Eq(true));
    ASSERT_THAT(state.StateWhenIdle, Eq(CommTransmitterOn));
    ASSERT_THAT(state.TransmitterBitRate, Eq(Comm9600bps));
}

TEST_F(CommTest, TestGetBaseLineTransmitterResponse)
{
    CommTransmitterState state;
    EXPECT_CALL(i2c, I2CWriteRead(TransmitterAddress, TransmitterGetState, Ne(nullptr), 1, Ne(nullptr), 1))
        .WillOnce(Invoke([](uint8_t /*address*/,
            uint8_t /*command*/,
            const uint8_t* /*inData*/,
            uint16_t /*inLength*/,
            uint8_t* outData,
            uint16_t /*outLength*/) {
            *outData = 0x0;
            return I2CResultOK;
        }));
    const auto status = comm.GetTransmitterState(state);
    ASSERT_THAT(status, Eq(true));
    ASSERT_THAT(state.BeaconState, Eq(false));
    ASSERT_THAT(state.StateWhenIdle, Eq(CommTransmitterOff));
    ASSERT_THAT(state.TransmitterBitRate, Eq(Comm1200bps));
}

TEST_F(CommTest, TestGetMixedLineTransmitterResponse)
{
    CommTransmitterState state;
    EXPECT_CALL(i2c, I2CWriteRead(TransmitterAddress, TransmitterGetState, Ne(nullptr), 1, Ne(nullptr), 1))
        .WillOnce(Invoke([](uint8_t /*address*/,
            uint8_t /*command*/,
            const uint8_t* /*inData*/,
            uint16_t /*inLength*/,
            uint8_t* outData,
            uint16_t /*outLength*/) {
            *outData = 0x0a;
            return I2CResultOK;
        }));
    const auto status = comm.GetTransmitterState(state);
    ASSERT_THAT(status, Eq(true));
    ASSERT_THAT(state.BeaconState, Eq(true));
    ASSERT_THAT(state.StateWhenIdle, Eq(CommTransmitterOff));
    ASSERT_THAT(state.TransmitterBitRate, Eq(Comm4800bps));
}

TEST_F(CommTest, TestSendTooLongFrame)
{
    uint8_t buffer[COMM_MAX_FRAME_CONTENTS_SIZE + 1] = {0};
    const auto status = comm.SendFrame(span<const uint8_t, COMM_MAX_FRAME_CONTENTS_SIZE + 1>(buffer));
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestSendFrameFailure)
{
    uint8_t buffer[10] = {0};
    EXPECT_CALL(i2c, I2CWriteRead(TransmitterAddress, TransmitterSendFrame, Ne(nullptr), COUNT_OF(buffer) + 1, _, _))
        .WillOnce(Return(I2CResultNack));
    const auto status = comm.SendFrame(span<const uint8_t, 10>(buffer));
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestSendFrame)
{
    uint8_t buffer[] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc};
    EXPECT_CALL(i2c, I2CWriteRead(TransmitterAddress, TransmitterSendFrame, Ne(nullptr), COUNT_OF(buffer) + 1, Ne(nullptr), 1))
        .WillOnce(Invoke(
            [](uint8_t /*address*/, uint8_t /*command*/, const uint8_t* inData, uint16_t length, uint8_t* outData, uint16_t /*outLength*/) {
                const uint8_t expected[] = {TransmitterSendFrame, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc};
                EXPECT_THAT(std::equal(inData, inData + length, std::begin(expected), std::end(expected)), Eq(true));
                *outData = 0;
                return I2CResultOK;
            }));
    const auto status = comm.SendFrame(span<const uint8_t>(buffer));
    ASSERT_THAT(status, Eq(true));
}

TEST_F(CommTest, TestSendFrameRejectedByHardware)
{
    uint8_t buffer[] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc};
    EXPECT_CALL(i2c, I2CWriteRead(TransmitterAddress, TransmitterSendFrame, _, _, Ne(nullptr), 1))
        .WillOnce(Invoke([](uint8_t /*address*/,
            uint8_t /*command*/,
            const uint8_t* /*inData*/,
            uint16_t /*length*/,
            uint8_t* outData,
            uint16_t /*outLength*/) {
            *outData = 0xff;
            return I2CResultOK;
        }));
    const auto status = comm.SendFrame(span<const uint8_t>(buffer));
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestReceiveFrameFailure)
{
    CommFrame frame;
    EXPECT_CALL(i2c, I2CWriteRead(ReceiverAddress, ReceiverGetFrame, _, _, Ne(nullptr), Ge(COMM_MAX_FRAME_CONTENTS_SIZE)))
        .WillOnce(Invoke([](uint8_t /*address*/,
            uint8_t /*command*/,
            const uint8_t* /*inData*/,
            uint16_t /*length*/,
            uint8_t* /*outData*/,
            uint16_t /*outLength*/) { return I2CResultNack; }));
    const auto status = comm.ReceiveFrame(frame);
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestReceiveFrameDopplerFrequencyOutOfRange)
{
    CommFrame frame;
    EXPECT_CALL(i2c, I2CWriteRead(ReceiverAddress, ReceiverGetFrame, _, _, Ne(nullptr), Ge(COMM_MAX_FRAME_CONTENTS_SIZE)))
        .WillOnce(Invoke([](uint8_t /*address*/,
            uint8_t /*command*/,
            const uint8_t* /*inData*/,
            uint16_t /*length*/,
            uint8_t* outData,
            uint16_t outLength) {
            memset(outData, 0, outLength);
            outData[0] = 1;
            outData[3] = 0xf0;
            return I2CResultOK;
        }));
    const auto status = comm.ReceiveFrame(frame);
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestReceiveFrameRSSIOutOfRange)
{
    CommFrame frame;
    EXPECT_CALL(i2c, I2CWriteRead(ReceiverAddress, ReceiverGetFrame, _, _, Ne(nullptr), Ge(COMM_MAX_FRAME_CONTENTS_SIZE)))
        .WillOnce(Invoke([](uint8_t /*address*/,
            uint8_t /*command*/,
            const uint8_t* /*inData*/,
            uint16_t /*length*/,
            uint8_t* outData,
            uint16_t outLength) {
            memset(outData, 0, outLength);
            outData[0] = 1;
            outData[5] = 0xf0;
            return I2CResultOK;
        }));
    const auto status = comm.ReceiveFrame(frame);
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestReceiveFrameSizeOutOfRange)
{
    CommFrame frame;
    EXPECT_CALL(i2c, I2CWriteRead(ReceiverAddress, ReceiverGetFrame, _, _, Ne(nullptr), Ge(COMM_MAX_FRAME_CONTENTS_SIZE)))
        .WillOnce(Invoke([](uint8_t /*address*/,
            uint8_t /*command*/,
            const uint8_t* /*inData*/,
            uint16_t /*length*/,
            uint8_t* outData,
            uint16_t outLength) {
            memset(outData, 0, outLength);
            outData[0] = 0xff;
            return I2CResultOK;
        }));
    const auto status = comm.ReceiveFrame(frame);
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestReceiveFrameSizeIsZero)
{
    CommFrame frame;
    EXPECT_CALL(i2c, I2CWriteRead(ReceiverAddress, ReceiverGetFrame, _, _, Ne(nullptr), Ge(COMM_MAX_FRAME_CONTENTS_SIZE)))
        .WillOnce(Invoke([](uint8_t /*address*/,
            uint8_t /*command*/,
            const uint8_t* /*inData*/,
            uint16_t /*length*/,
            uint8_t* outData,
            uint16_t outLength) {
            memset(outData, 0, outLength);
            return I2CResultOK;
        }));
    const auto status = comm.ReceiveFrame(frame);
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestReceiveFrameSizeIsOutOfRange)
{
    CommFrame frame;
    EXPECT_CALL(i2c, I2CWriteRead(ReceiverAddress, ReceiverGetFrame, _, _, Ne(nullptr), Ge(COMM_MAX_FRAME_CONTENTS_SIZE)))
        .WillOnce(Invoke([](uint8_t /*address*/,
            uint8_t /*command*/,
            const uint8_t* /*inData*/,
            uint16_t /*length*/,
            uint8_t* outData,
            uint16_t outLength) {
            memset(outData, 0, outLength);
            outData[0] = COMM_MAX_FRAME_CONTENTS_SIZE + 1;
            return I2CResultOK;
        }));
    const auto status = comm.ReceiveFrame(frame);
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestReceiveFrame)
{
    CommFrame frame;
    const uint8_t expected[] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
    EXPECT_CALL(i2c, I2CWriteRead(ReceiverAddress, ReceiverGetFrame, _, _, Ne(nullptr), Ge(COUNT_OF(expected) + 6)))
        .WillOnce(Invoke([&](uint8_t /*address*/,
            uint8_t /*command*/,
            const uint8_t* /*inData*/,
            uint16_t /*length*/,
            uint8_t* outData,
            uint16_t outLength) {

            memset(outData, 0, outLength);
            outData[0] = COUNT_OF(expected);
            outData[2] = 0xab;
            outData[3] = 0x0c;
            outData[4] = 0xde;
            outData[5] = 0x0d;
            memcpy(outData + 6, expected, COUNT_OF(expected));
            return I2CResultOK;
        }));
    const auto status = comm.ReceiveFrame(frame);
    ASSERT_THAT(status, Eq(true));
    ASSERT_THAT(frame.Size, Eq(COUNT_OF(expected)));
    ASSERT_THAT(frame.Doppler, Eq(0xcab));
    ASSERT_THAT(frame.RSSI, Eq(0xdde));
    ASSERT_TRUE(std::equal(expected, expected + COUNT_OF(expected), frame.Contents, frame.Contents + frame.Size));
}

TEST_F(CommTest, TestReceiverTelemetry)
{
    CommReceiverTelemetry telemetry;
    const uint8_t expected[] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e};
    EXPECT_CALL(i2c, I2CWriteRead(ReceiverAddress, ReceverGetTelemetry, _, _, Ne(nullptr), Ge(sizeof(CommReceiverTelemetry))))
        .WillOnce(Invoke([&](uint8_t /*address*/,
            uint8_t /*command*/,
            const uint8_t* /*inData*/,
            uint16_t /*length*/,
            uint8_t* outData,
            uint16_t outLength) {
            memset(outData, 0, outLength);
            memcpy(outData, expected, COUNT_OF(expected));
            return I2CResultOK;
        }));
    const auto status = comm.GetReceiverTelemetry(telemetry);
    ASSERT_THAT(status, Eq(true));
    ASSERT_THAT(telemetry.TransmitterCurrentConsumption, Eq(0x0201));
    ASSERT_THAT(telemetry.DopplerOffset, Eq(0x0403));
    ASSERT_THAT(telemetry.ReceiverCurrentConsumption, Eq(0x0605));
    ASSERT_THAT(telemetry.Vcc, Eq(0x0807));
    ASSERT_THAT(telemetry.OscilatorTemperature, Eq(0x0a09));
    ASSERT_THAT(telemetry.AmplifierTemperature, Eq(0x0c0b));
    ASSERT_THAT(telemetry.SignalStrength, Eq(0x0e0d));
}

TEST_F(CommTest, TestTransmitterTelemetry)
{
    CommTransmitterTelemetry telemetry;
    const uint8_t expected[] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8};
    EXPECT_CALL(i2c, I2CWriteRead(TransmitterAddress, TransmitterGetTelemetry, _, _, Ne(nullptr), Ge(sizeof(CommTransmitterTelemetry))))
        .WillOnce(Invoke([&](uint8_t /*address*/,
            uint8_t /*command*/,
            const uint8_t* /*inData*/,
            uint16_t /*length*/,
            uint8_t* outData,
            uint16_t outLength) {
            memset(outData, 0, outLength);
            memcpy(outData, expected, COUNT_OF(expected));
            return I2CResultOK;
        }));
    const auto status = comm.GetTransmitterTelemetry(telemetry);
    ASSERT_THAT(status, Eq(true));
    ASSERT_THAT(telemetry.RFReflectedPower, Eq(0x0201));
    ASSERT_THAT(telemetry.AmplifierTemperature, Eq(0x0403));
    ASSERT_THAT(telemetry.RFForwardPower, Eq(0x0605));
    ASSERT_THAT(telemetry.TransmitterCurrentConsumption, Eq(0x0807));
}

TEST_F(CommTest, TestSetBeaconFailure)
{
    CommBeacon beacon;
    memset(&beacon, 0, sizeof(beacon));
    beacon.DataSize = 1;
    EXPECT_CALL(i2c, I2CWrite(TransmitterAddress, TransmitterSetBeacon, Ne(nullptr), _)).WillOnce(Return(I2CResultNack));
    const auto status = comm.SetBeacon(beacon);
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestSetBeaconSizeOutOfRange)
{
    CommBeacon beacon;
    memset(&beacon, 0, sizeof(beacon));
    beacon.DataSize = COMM_MAX_FRAME_CONTENTS_SIZE + 1;
    EXPECT_CALL(i2c, I2CWrite(TransmitterAddress, TransmitterSetBeacon, Ne(nullptr), _)).Times(0);
    const auto status = comm.SetBeacon(beacon);
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestSetBeacon)
{
    const uint8_t data[] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8};
    CommBeacon beacon;
    memset(&beacon, 0, sizeof(beacon));
    beacon.DataSize = COUNT_OF(data);
    beacon.Period = 0x0a0b;
    memcpy(beacon.Data, data, COUNT_OF(data));
    EXPECT_CALL(i2c, I2CWrite(TransmitterAddress, TransmitterSetBeacon, Ne(nullptr), Eq(COUNT_OF(data) + 3)))
        .WillOnce(Invoke([](uint8_t /*address*/, uint8_t /*command*/, const uint8_t* inData, uint16_t length) {
            const uint8_t expected[] = {TransmitterSetBeacon, 0x0b, 0x0a, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8};
            EXPECT_THAT(std::equal(inData, inData + length, std::begin(expected), std::end(expected)), Eq(true));
            return I2CResultOK;
        }));
    const auto status = comm.SetBeacon(beacon);
    ASSERT_THAT(status, Eq(true));
}

TEST_F(CommTest, TestPauseNonExistingTask)
{
    OSMock system;
    auto guard = InstallProxy(&system);
    EXPECT_CALL(system, SuspendTask(_)).Times(0);
    const auto status = comm.Pause();
    ASSERT_THAT(status, Eq(true));
}

class CommReceiverTelemetryTest : public testing::TestWithParam<std::tuple<int, uint8_t, I2CResult>>
{
  public:
    CommReceiverTelemetryTest();

  protected:
    CommObject comm;
    FrameHandlerMock frameHandler;
    I2CBusMock i2c;
    testing::NiceMock<OSMock> system;
    OSReset reset;
};

CommReceiverTelemetryTest::CommReceiverTelemetryTest() : comm(i2c, frameHandler)
{
    reset = SetupComm(comm, system);
}

TEST_P(CommReceiverTelemetryTest, TestInvalidTelemetry)
{
    CommReceiverTelemetry telemetry;
    const auto index = std::get<0>(GetParam());
    const auto value = std::get<1>(GetParam());
    const auto operationStatus = std::get<2>(GetParam());
    EXPECT_CALL(i2c, I2CWriteRead(ReceiverAddress, ReceverGetTelemetry, _, _, Ne(nullptr), Ge(sizeof(CommReceiverTelemetry))))
        .WillOnce(Invoke([&](uint8_t /*address*/,
            uint8_t /*command*/,
            const uint8_t* /*inData*/,
            uint16_t /*length*/,
            uint8_t* outData,
            uint16_t outLength) {
            memset(outData, 0, outLength);
            outData[index] = value;
            return operationStatus;
        }));
    const auto status = comm.GetReceiverTelemetry(telemetry);
    ASSERT_THAT(status, Eq(false));
}

INSTANTIATE_TEST_CASE_P(CommReceiverTelemetryValuesOutOfRange,
    CommReceiverTelemetryTest,
    testing::Values(std::make_tuple(0, 0, I2CResultNack),
                            std::make_tuple(1, 0xf0, I2CResultOK),
                            std::make_tuple(3, 0xf0, I2CResultOK),
                            std::make_tuple(5, 0xf0, I2CResultOK),
                            std::make_tuple(7, 0xf0, I2CResultOK),
                            std::make_tuple(9, 0xf0, I2CResultOK),
                            std::make_tuple(11, 0xf0, I2CResultOK),
                            std::make_tuple(13, 0xf0, I2CResultOK)), );

class CommTransmitterTelemetryTest : public testing::TestWithParam<std::tuple<int, uint8_t, I2CResult>>
{
  public:
    CommTransmitterTelemetryTest();

  protected:
    CommObject comm;
    FrameHandlerMock frameHandler;
    I2CBusMock i2c;
    testing::NiceMock<OSMock> system;
    OSReset reset;
};

CommTransmitterTelemetryTest::CommTransmitterTelemetryTest() : comm(i2c, frameHandler)
{
    reset = SetupComm(comm, system);
}

TEST_P(CommTransmitterTelemetryTest, TestInvalidTelemetry)
{
    CommTransmitterTelemetry telemetry;
    const auto index = std::get<0>(GetParam());
    const auto value = std::get<1>(GetParam());
    const auto operationStatus = std::get<2>(GetParam());
    EXPECT_CALL(i2c, I2CWriteRead(TransmitterAddress, TransmitterGetTelemetry, _, _, Ne(nullptr), Ge(sizeof(CommTransmitterTelemetry))))
        .WillOnce(Invoke([&](uint8_t /*address*/,
            uint8_t /*command*/,
            const uint8_t* /*inData*/,
            uint16_t /*length*/,
            uint8_t* outData,
            uint16_t outLength) {
            memset(outData, 0, outLength);
            outData[index] = value;
            return operationStatus;
        }));
    const auto status = comm.GetTransmitterTelemetry(telemetry);
    ASSERT_THAT(status, Eq(false));
}

INSTANTIATE_TEST_CASE_P(CommTransmitterTelemetryValuesOutOfRange,
    CommTransmitterTelemetryTest,
    testing::Values(std::make_tuple(0, 0, I2CResultNack),
                            std::make_tuple(1, 0xf0, I2CResultOK),
                            std::make_tuple(3, 0xf0, I2CResultOK),
                            std::make_tuple(5, 0xf0, I2CResultOK),
                            std::make_tuple(7, 0xf0, I2CResultOK)), );
