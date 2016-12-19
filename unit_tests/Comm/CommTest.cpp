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
#include "utils.hpp"

using testing::_;
using testing::Eq;
using testing::Ne;
using testing::Ge;
using testing::StrEq;
using testing::Return;
using testing::Invoke;
using testing::Pointee;
using testing::ElementsAre;
using testing::Matches;
using gsl::span;
using drivers::i2c::I2CResult;
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
    MOCK_METHOD2(HandleFrame, void(ITransmitFrame&, CommFrame&));
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
    std::uint8_t dataBuffer[devices::comm::ComPrefferedBufferSize];
};

CommTest::CommTest() : comm(i2c, frameHandler)
{
    reset = SetupComm(comm, system);
}

TEST_F(CommTest, TestInitializationDoesNotTouchHardware)
{
    CommObject commObject(i2c, frameHandler);

    EXPECT_CALL(i2c, WriteRead(_, _, _)).Times(0);
    EXPECT_CALL(i2c, Write(_, _)).Times(0);

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
    i2c.ExpectWriteCommand(ReceiverAddress, HardwareReset).WillOnce(Return(I2CResult::OK));
    i2c.ExpectWriteCommand(TransmitterAddress, TransmitterReset).WillOnce(Return(I2CResult::OK));
    i2c.ExpectWriteCommand(ReceiverAddress, ReceiverReset).WillOnce(Return(I2CResult::OK));
    const auto status = comm.Reset();
    ASSERT_THAT(status, Eq(true));
}

TEST_F(CommTest, TestHardwareResetFailureOnHardware)
{
    i2c.ExpectWriteCommand(ReceiverAddress, HardwareReset).WillOnce(Return(I2CResult::Nack));
    const auto status = comm.Reset();
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestHardwareResetFailureOnReceiver)
{
    i2c.ExpectWriteCommand(ReceiverAddress, HardwareReset).WillOnce(Return(I2CResult::OK));
    i2c.ExpectWriteCommand(ReceiverAddress, ReceiverReset).WillOnce(Return(I2CResult::Nack));
    ON_CALL(i2c, Write(TransmitterAddress, _)).WillByDefault(Return(I2CResult::OK));
    const auto status = comm.Reset();
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestHardwareResetFailureOnTransmitter)
{
    i2c.ExpectWriteCommand(TransmitterAddress, TransmitterReset).WillOnce(Return(I2CResult::Nack));
    EXPECT_CALL(i2c, Write(ReceiverAddress, _)).WillRepeatedly(Return(I2CResult::OK));
    const auto status = comm.Reset();
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestTransmitterReset)
{
    i2c.ExpectWriteCommand(TransmitterAddress, TransmitterReset).WillOnce(Return(I2CResult::OK));
    const auto status = comm.ResetTransmitter();
    ASSERT_THAT(status, Eq(true));
}

TEST_F(CommTest, TestTransmitterResetFailure)
{
    i2c.ExpectWriteCommand(TransmitterAddress, TransmitterReset).WillOnce(Return(I2CResult::Nack));
    const auto status = comm.ResetTransmitter();
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestReceiverReset)
{
    i2c.ExpectWriteCommand(ReceiverAddress, ReceiverReset).WillOnce(Return(I2CResult::OK));
    const auto status = comm.ResetReceiver();
    ASSERT_THAT(status, Eq(true));
}

TEST_F(CommTest, TestReceiverResetFailure)
{
    i2c.ExpectWriteCommand(ReceiverAddress, ReceiverReset).WillOnce(Return(I2CResult::Nack));
    const auto status = comm.ResetReceiver();
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestFrameRemoval)
{
    i2c.ExpectWriteCommand(ReceiverAddress, ReceiverRemoveFrame).WillOnce(Return(I2CResult::OK));
    const auto status = comm.RemoveFrame();
    ASSERT_THAT(status, Eq(true));
}

TEST_F(CommTest, TestFrameRemovalFailure)
{
    i2c.ExpectWriteCommand(ReceiverAddress, ReceiverRemoveFrame).WillOnce(Return(I2CResult::Nack));
    const auto status = comm.RemoveFrame();
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestGetFrameCountFailure)
{
    EXPECT_CALL(i2c, WriteRead(ReceiverAddress, ElementsAre(ReceiverGetFrameCount), _)).WillOnce(Return(I2CResult::Nack));
    const auto result = comm.GetFrameCount();
    ASSERT_THAT(result.status, Eq(false));
    ASSERT_THAT(result.frameCount, Eq(0));
}

TEST_F(CommTest, TestGetFrameCount)
{
    EXPECT_CALL(i2c, WriteRead(ReceiverAddress, ElementsAre(ReceiverGetFrameCount), _))
        .WillOnce(Invoke([](uint8_t /*address*/, auto /*inData*/, auto outData) {
            outData[0] = 31;
            return I2CResult::OK;
        }));

    const auto result = comm.GetFrameCount();
    ASSERT_THAT(result.status, Eq(true));
    ASSERT_THAT(result.frameCount, Eq(31));
}

TEST_F(CommTest, TestClearBeaconFailure)
{
    i2c.ExpectWriteCommand(TransmitterAddress, TransmitterClearBeacon).WillOnce(Return(I2CResult::Nack));
    const auto status = comm.ClearBeacon();
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestClearBeacon)
{
    i2c.ExpectWriteCommand(TransmitterAddress, TransmitterClearBeacon).WillOnce(Return(I2CResult::OK));
    const auto status = comm.ClearBeacon();
    ASSERT_THAT(status, Eq(true));
}

TEST_F(CommTest, TestSetIdleStateFailure)
{
    EXPECT_CALL(i2c, Write(TransmitterAddress, ElementsAre(TransmitterSetIdleState, _))).WillOnce(Return(I2CResult::Nack));
    const auto status = comm.SetTransmitterStateWhenIdle(CommTransmitterOn);
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestSetIdleState)
{
    EXPECT_CALL(i2c, Write(TransmitterAddress, ElementsAre(TransmitterSetIdleState, _))).WillOnce(Return(I2CResult::OK));
    const auto status = comm.SetTransmitterStateWhenIdle(CommTransmitterOn);
    ASSERT_THAT(status, Eq(true));
}

TEST_F(CommTest, TestSetIdleStateCommandOn)
{
    EXPECT_CALL(i2c, Write(TransmitterAddress, ElementsAre(TransmitterSetIdleState, _)))
        .WillOnce(Invoke([](uint8_t /*address*/, auto inData) {
            EXPECT_THAT(inData[1], Eq(1));
            return I2CResult::OK;
        }));
    const auto status = comm.SetTransmitterStateWhenIdle(CommTransmitterOn);
    ASSERT_THAT(status, Eq(true));
}

TEST_F(CommTest, TestSetIdleStateCommandOff)
{
    EXPECT_CALL(i2c, Write(TransmitterAddress, ElementsAre(TransmitterSetIdleState, _)))
        .WillOnce(Invoke([](uint8_t /*address*/, auto inData) {
            EXPECT_THAT(inData[1], Eq(0));
            return I2CResult::OK;
        }));
    const auto status = comm.SetTransmitterStateWhenIdle(CommTransmitterOff);
    ASSERT_THAT(status, Eq(true));
}

TEST_F(CommTest, TestSetBitRateFailure)
{
    EXPECT_CALL(i2c, Write(TransmitterAddress, ElementsAre(TransmitterSetBitRate, _))).WillOnce(Return(I2CResult::Nack));
    const auto status = comm.SetTransmitterBitRate(Comm1200bps);
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestSetBitRate)
{
    EXPECT_CALL(i2c, Write(TransmitterAddress, ElementsAre(TransmitterSetBitRate, _))).WillOnce(Return(I2CResult::OK));
    const auto status = comm.SetTransmitterBitRate(Comm1200bps);
    ASSERT_THAT(status, Eq(true));
}

TEST_F(CommTest, TestSetBitRateCommand)
{
    EXPECT_CALL(i2c, Write(TransmitterAddress, ElementsAre(TransmitterSetBitRate, _)))
        .WillOnce(Invoke([](uint8_t /*address*/, auto inData) {
            EXPECT_THAT(inData[1], Eq(1));
            return I2CResult::OK;
        }))
        .WillOnce(Invoke([](uint8_t /*address*/, auto inData) {
            EXPECT_THAT(inData[1], Eq(2));
            return I2CResult::OK;
        }))
        .WillOnce(Invoke([](uint8_t /*address*/, auto inData) {
            EXPECT_THAT(inData[1], Eq(4));
            return I2CResult::OK;
        }))
        .WillOnce(Invoke([](uint8_t /*address*/, auto inData) {
            EXPECT_THAT(inData[1], Eq(8));
            return I2CResult::OK;
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
    EXPECT_CALL(i2c, WriteRead(TransmitterAddress, ElementsAre(TransmitterGetState), _)).WillOnce(Return(I2CResult::Nack));
    const auto status = comm.GetTransmitterState(state);
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestGetTransmitterInvalidResponse)
{
    CommTransmitterState state;
    EXPECT_CALL(i2c, WriteRead(TransmitterAddress, ElementsAre(TransmitterGetState), _))
        .WillOnce(Invoke([](uint8_t /*address*/, auto /*inData*/, auto outData) {
            outData[0] = 0xff;
            return I2CResult::OK;
        }));
    const auto status = comm.GetTransmitterState(state);
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestGetTransmitterResponse)
{
    CommTransmitterState state;
    EXPECT_CALL(i2c, WriteRead(TransmitterAddress, ElementsAre(TransmitterGetState), _))
        .WillOnce(Invoke([](uint8_t /*address*/, auto /*inData*/, auto outData) {
            outData[0] = 0x7f;
            return I2CResult::OK;
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
    EXPECT_CALL(i2c, WriteRead(TransmitterAddress, ElementsAre(TransmitterGetState), _))
        .WillOnce(Invoke([](uint8_t /*address*/, auto /*inData*/, auto outData) {
            outData[0] = 0x0;
            return I2CResult::OK;
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
    EXPECT_CALL(i2c, WriteRead(TransmitterAddress, ElementsAre(TransmitterGetState), _))
        .WillOnce(Invoke([](uint8_t /*address*/, auto /*inData*/, auto outData) {
            outData[0] = 0x0a;
            return I2CResult::OK;
        }));
    const auto status = comm.GetTransmitterState(state);
    ASSERT_THAT(status, Eq(true));
    ASSERT_THAT(state.BeaconState, Eq(true));
    ASSERT_THAT(state.StateWhenIdle, Eq(CommTransmitterOff));
    ASSERT_THAT(state.TransmitterBitRate, Eq(Comm4800bps));
}

TEST_F(CommTest, TestSendTooLongFrame)
{
    uint8_t buffer[devices::comm::CommMaxFrameSize + 1] = {0};
    const auto status = comm.SendFrame(buffer);
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestSendFrameFailure)
{
    uint8_t buffer[10] = {0};
    EXPECT_CALL(i2c, WriteRead(TransmitterAddress, BeginsWith(TransmitterSendFrame), _)).WillOnce(Return(I2CResult::Nack));
    const auto status = comm.SendFrame(span<const uint8_t, 10>(buffer));
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestSendFrame)
{
    uint8_t buffer[] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc};
    EXPECT_CALL(i2c, WriteRead(TransmitterAddress, BeginsWith(TransmitterSendFrame), _))
        .WillOnce(Invoke([](uint8_t /*address*/, span<const uint8_t> inData, span<uint8_t> outData) {
            const uint8_t expected[] = {TransmitterSendFrame, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc};
            //            EXPECT_THAT(std::equal(inData.begin(), inData.end(), std::begin(expected), std::end(expected)), Eq(true));

            EXPECT_THAT(inData, Eq(span<const uint8_t>(expected)));

            outData[0] = 0;
            return I2CResult::OK;
        }));
    const auto status = comm.SendFrame(span<const uint8_t>(buffer));
    ASSERT_THAT(status, Eq(true));
}

TEST_F(CommTest, TestSendFrameRejectedByHardware)
{
    uint8_t buffer[] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc};
    EXPECT_CALL(i2c, WriteRead(TransmitterAddress, BeginsWith(TransmitterSendFrame), _))
        .WillOnce(Invoke([](uint8_t /*address*/, span<const uint8_t> /*inData*/, span<uint8_t> outData) {
            outData[0] = 0xff;
            return I2CResult::OK;
        }));
    const auto status = comm.SendFrame(span<const uint8_t>(buffer));
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestReceiveFrameFailure)
{
    CommFrame frame;
    EXPECT_CALL(i2c, WriteRead(ReceiverAddress, ElementsAre(ReceiverGetFrame), _)).WillOnce(Return(I2CResult::Nack));
    const auto status = comm.ReceiveFrame(dataBuffer, frame);
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestReceiveFrameTooShortBuffer)
{
    std::uint8_t buffer[1];
    CommFrame frame;
    EXPECT_CALL(i2c, WriteRead(ReceiverAddress, ElementsAre(ReceiverGetFrame), _)).Times(0);
    const auto status = comm.ReceiveFrame(buffer, frame);
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestReceiveFramePartialHeader)
{
    std::uint8_t buffer[5];
    CommFrame frame;
    EXPECT_CALL(i2c, WriteRead(ReceiverAddress, ElementsAre(ReceiverGetFrame), _))
        .WillRepeatedly(Invoke([&](uint8_t /*address*/, span<const uint8_t> /*inData*/, span<uint8_t> outData) {
            std::fill(outData.begin(), outData.end(), 0);
            outData[0] = 32;
            if (static_cast<size_t>(outData.size()) > 4)
            {
                outData[2] = 0xab;
                outData[3] = 0x0c;
                outData[4] = 0xde;
            }

            return I2CResult::OK;
        }));
    const auto status = comm.ReceiveFrame(buffer, frame);
    ASSERT_THAT(status, Eq(true));
    ASSERT_THAT(frame.FullSize(), Eq(32));
    ASSERT_THAT(frame.Doppler(), Eq(0xcab));
    ASSERT_THAT(frame.Rssi(), Eq(0));
    ASSERT_THAT(frame.Payload().empty(), Eq(true));
}

TEST_F(CommTest, TestReceiveFrameReceiveFailure)
{
    CommFrame frame;
    EXPECT_CALL(i2c, WriteRead(ReceiverAddress, ElementsAre(ReceiverGetFrame), _))
        .WillOnce(Invoke([](uint8_t /*address*/, span<const uint8_t> /*inData*/, span<uint8_t> outData) {
            std::fill(outData.begin(), outData.end(), 0);
            outData[0] = 1;
            return I2CResult::OK;
        }))
        .WillOnce(Return(I2CResult::Nack));
    const auto status = comm.ReceiveFrame(dataBuffer, frame);
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestReceiveFrame)
{
    CommFrame frame;
    const uint8_t expected[] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
    EXPECT_CALL(i2c, WriteRead(ReceiverAddress, ElementsAre(ReceiverGetFrame), _))
        .WillRepeatedly(Invoke([&](uint8_t /*address*/, span<const uint8_t> /*inData*/, span<uint8_t> outData) {
            std::fill(outData.begin(), outData.end(), 0);
            outData[0] = COUNT_OF(expected);
            if (static_cast<size_t>(outData.size()) > (5 + count_of(expected)))
            {
                outData[2] = 0xab;
                outData[3] = 0x0c;
                outData[4] = 0xde;
                outData[5] = 0x0d;
                std::copy(std::begin(expected), std::end(expected), outData.begin() + 6);
            }

            return I2CResult::OK;
        }));
    const auto status = comm.ReceiveFrame(dataBuffer, frame);
    ASSERT_THAT(status, Eq(true));
    ASSERT_THAT(frame.Size(), Eq(COUNT_OF(expected)));
    ASSERT_THAT(frame.Doppler(), Eq(0xcab));
    ASSERT_THAT(frame.Rssi(), Eq(0xdde));
    ASSERT_THAT(gsl::span<const std::uint8_t>(frame.Payload()), Eq(span<const uint8_t>(expected)));
}

TEST_F(CommTest, TestReceiveFramePartialData)
{
    std::uint8_t buffer[22];
    CommFrame frame;
    const uint8_t expected[] = {0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
    EXPECT_CALL(i2c, WriteRead(ReceiverAddress, ElementsAre(ReceiverGetFrame), _))
        .WillRepeatedly(Invoke([&](uint8_t /*address*/, span<const uint8_t> /*inData*/, span<uint8_t> outData) {
            std::fill(outData.begin(), outData.end(), 0);
            outData[0] = 32;
            if (static_cast<size_t>(outData.size()) > 5)
            {
                outData[2] = 0xab;
                outData[3] = 0x0c;
                outData[4] = 0xde;
                outData[5] = 0x0d;
            }

            auto contentSize = std::min<ptrdiff_t>(outData.size() - 6, count_of(expected));
            if (contentSize > 0)
            {
                std::copy(std::begin(expected), std::begin(expected) + contentSize, outData.begin() + 6);
            }

            return I2CResult::OK;
        }));
    const auto status = comm.ReceiveFrame(buffer, frame);
    ASSERT_THAT(status, Eq(true));
    ASSERT_THAT(frame.Size(), Eq(16));
    ASSERT_THAT(frame.FullSize(), Eq(32));
    ASSERT_THAT(frame.Doppler(), Eq(0xcab));
    ASSERT_THAT(frame.Rssi(), Eq(0xdde));
    ASSERT_THAT(gsl::span<const std::uint8_t>(frame.Payload()), Eq(span<const uint8_t>(expected)));
}

TEST_F(CommTest, TestReceiverTelemetry)
{
    CommReceiverTelemetry telemetry;
    const uint8_t expected[] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e};
    EXPECT_CALL(i2c, WriteRead(ReceiverAddress, ElementsAre(ReceverGetTelemetry), _))
        .WillOnce(Invoke([&](uint8_t /*address*/, span<const uint8_t> /*inData*/, span<uint8_t> outData) {
            std::fill(outData.begin(), outData.end(), 0);
            std::copy(std::begin(expected), std::end(expected), outData.begin());
            return I2CResult::OK;
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
    EXPECT_CALL(i2c, WriteRead(TransmitterAddress, ElementsAre(TransmitterGetTelemetry), _))
        .WillOnce(Invoke([&](uint8_t /*address*/, span<const uint8_t> /*inData*/, span<uint8_t> outData) {
            std::fill(outData.begin(), outData.end(), 0);
            std::copy(std::begin(expected), std::end(expected), outData.begin());
            return I2CResult::OK;
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
    EXPECT_CALL(i2c, Write(TransmitterAddress, BeginsWith(TransmitterSetBeacon))).WillOnce(Return(I2CResult::Nack));
    const auto status = comm.SetBeacon(beacon);
    ASSERT_THAT(status, Eq(false));
}

TEST_F(CommTest, TestSetBeaconSizeOutOfRange)
{
    CommBeacon beacon;
    memset(&beacon, 0, sizeof(beacon));
    beacon.DataSize = devices::comm::CommMaxFrameSize + 1;
    EXPECT_CALL(i2c, Write(TransmitterAddress, BeginsWith(TransmitterSetBeacon))).Times(0);
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
    EXPECT_CALL(i2c, Write(TransmitterAddress, BeginsWith(TransmitterSetBeacon)))
        .WillOnce(Invoke([](uint8_t /*address*/, span<const uint8_t> inData) {
            const uint8_t expected[] = {TransmitterSetBeacon, 0x0b, 0x0a, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8};
            EXPECT_THAT(inData, Eq(span<const uint8_t>(expected)));
            return I2CResult::OK;
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

    UNUSED(index, value, operationStatus);

    EXPECT_CALL(i2c, WriteRead(ReceiverAddress, ElementsAre(ReceverGetTelemetry), _))
        .WillOnce(Invoke([&](uint8_t /*address*/, span<const uint8_t> /*inData*/, span<uint8_t> outData) {
            std::fill(outData.begin(), outData.end(), 0);
            outData[index] = value;
            return operationStatus;
        }));
    const auto status = comm.GetReceiverTelemetry(telemetry);
    ASSERT_THAT(status, Eq(false));
}

INSTANTIATE_TEST_CASE_P(CommReceiverTelemetryValuesOutOfRange,
    CommReceiverTelemetryTest,
    testing::Values(std::make_tuple(0, 0, I2CResult::Nack),
        std::make_tuple(1, 0xf0, I2CResult::OK),
        std::make_tuple(3, 0xf0, I2CResult::OK),
        std::make_tuple(5, 0xf0, I2CResult::OK),
        std::make_tuple(7, 0xf0, I2CResult::OK),
        std::make_tuple(9, 0xf0, I2CResult::OK),
        std::make_tuple(11, 0xf0, I2CResult::OK),
        std::make_tuple(13, 0xf0, I2CResult::OK)), );

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
    UNUSED(index, value, operationStatus);
    EXPECT_CALL(i2c, WriteRead(TransmitterAddress, ElementsAre(TransmitterGetTelemetry), _))
        .WillOnce(Invoke([=](uint8_t /*address*/, span<const uint8_t> /*inData*/, span<uint8_t> outData) {
            std::fill(outData.begin(), outData.end(), 0);
            outData[index] = value;
            return operationStatus;
        }));
    const auto status = comm.GetTransmitterTelemetry(telemetry);
    ASSERT_THAT(status, Eq(false));
}

INSTANTIATE_TEST_CASE_P(CommTransmitterTelemetryValuesOutOfRange,
    CommTransmitterTelemetryTest,
    testing::Values(std::make_tuple(0, 0, I2CResult::Nack),
        std::make_tuple(1, 0xf0, I2CResult::OK),
        std::make_tuple(3, 0xf0, I2CResult::OK),
        std::make_tuple(5, 0xf0, I2CResult::OK),
        std::make_tuple(7, 0xf0, I2CResult::OK)), );
