#include <algorithm>
#include <array>
#include <chrono>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "I2C/I2CMock.hpp"
#include "OsMock.hpp"
#include "base/reader.h"
#include "base/writer.h"
#include "mock/comm.hpp"
#include "obc/telecommands/i2c.hpp"
#include "telecommunication/downlink.h"
#include "telecommunication/telecommand_handling.h"

using testing::_;
using testing::Eq;
using testing::ElementsAreArray;
using testing::Invoke;
using testing::Return;

using telecommunication::downlink::DownlinkFrame;
using telecommunication::downlink::DownlinkAPID;
using drivers::i2c::I2CResult;

using gsl::span;

template <std::size_t Size> using Buffer = std::array<std::uint8_t, Size>;

namespace
{
    class RawI2CTelecommandTest : public testing::Test
    {
      protected:
        RawI2CTelecommandTest();

        testing::NiceMock<TransmitterMock> transmitter;
        testing::NiceMock<I2CBusMock> systemBus;
        testing::NiceMock<I2CBusMock> payload;

        testing::NiceMock<OSMock> os;
        OSReset guard;

        obc::telecommands::RawI2CTelecommand telecommand{systemBus, payload};
    };

    RawI2CTelecommandTest::RawI2CTelecommandTest()
    {
        this->guard = InstallProxy(&os);
    }

    TEST_F(RawI2CTelecommandTest, ShouldUseSystemBus)
    {
        const std::string data{"some information"};
        span<const uint8_t> dataSpan(reinterpret_cast<const uint8_t*>(data.data()), data.length());

        Buffer<DownlinkFrame::MaxPayloadSize> expectedPayload;
        expectedPayload.fill(0);
        Writer response(expectedPayload);
        response.WriteByte(0xFF);
        response.WriteByte(0x00);
        response.WriteArray(dataSpan);

        Buffer<3> dataSent;
        dataSent[0] = 0x01;
        dataSent[1] = 0x02;
        dataSent[2] = 0x03;

        EXPECT_CALL(transmitter, SendFrame(IsDownlinkFrame(Eq(DownlinkAPID::Operation), Eq(0U), ElementsAreArray(expectedPayload))));
        EXPECT_CALL(systemBus, Write(0x01, ElementsAreArray(dataSent))).WillOnce(Return(I2CResult::OK));
        EXPECT_CALL(systemBus, Read(0x01, _)).WillOnce(Invoke([=](uint8_t, span<uint8_t> outData) {
            std::copy(data.begin(), data.end(), outData.begin());
            return I2CResult::OK;
        }));
        EXPECT_CALL(os, Sleep(Eq(std::chrono::milliseconds{0x0203})));

        Buffer<200> buffer;
        Writer w(buffer);
        w.WriteByte(0xFF);
        w.WriteByte(0x00);
        w.WriteByte(0x01);
        w.WriteWordLE(0x0203);
        w.WriteArray(dataSent);

        telecommand.Handle(transmitter, w.Capture());
    }

    TEST_F(RawI2CTelecommandTest, ShouldUsePayload)
    {
        const std::string data{"response from payload"};
        span<const uint8_t> dataSpan(reinterpret_cast<const uint8_t*>(data.data()), data.length());

        Buffer<DownlinkFrame::MaxPayloadSize> expectedPayload;
        expectedPayload.fill(0);
        Writer response(expectedPayload);
        response.WriteByte(0xFF);
        response.WriteByte(0x00);
        response.WriteArray(dataSpan);

        Buffer<5> dataSent;
        dataSent[0] = 0x01;
        dataSent[1] = 0x02;
        dataSent[2] = 0x03;
        dataSent[3] = 0x04;
        dataSent[4] = 0x05;

        EXPECT_CALL(transmitter, SendFrame(IsDownlinkFrame(Eq(DownlinkAPID::Operation), Eq(0U), ElementsAreArray(expectedPayload))));
        EXPECT_CALL(payload, Write(0x33, ElementsAreArray(dataSent))).WillOnce(Return(I2CResult::OK));
        EXPECT_CALL(payload, Read(0x33, _)).WillOnce(Invoke([=](uint8_t, span<uint8_t> outData) {
            std::copy(data.begin(), data.end(), outData.begin());
            return I2CResult::OK;
        }));
        EXPECT_CALL(os, Sleep(Eq(std::chrono::milliseconds{0x9988})));

        Buffer<200> buffer;
        Writer w(buffer);
        w.WriteByte(0xFF);
        w.WriteByte(0x01);
        w.WriteByte(0x33);
        w.WriteWordLE(0x9988);
        w.WriteArray(dataSent);

        telecommand.Handle(transmitter, w.Capture());
    }

    TEST_F(RawI2CTelecommandTest, ShouldWriteAndReadWithoutDelay)
    {
        const std::string data{"response"};
        span<const uint8_t> dataSpan(reinterpret_cast<const uint8_t*>(data.data()), data.length());

        Buffer<DownlinkFrame::MaxPayloadSize> expectedPayload;
        expectedPayload.fill(0);
        Writer response(expectedPayload);
        response.WriteByte(0xFF);
        response.WriteByte(0x00);
        response.WriteArray(dataSpan);

        Buffer<5> dataSent;
        dataSent[0] = 0x0D;
        dataSent[1] = 0x0E;
        dataSent[2] = 0x0A;
        dataSent[3] = 0x0D;

        EXPECT_CALL(transmitter, SendFrame(IsDownlinkFrame(Eq(DownlinkAPID::Operation), Eq(0U), ElementsAreArray(expectedPayload))));
        EXPECT_CALL(systemBus, WriteRead(0x10, ElementsAreArray(dataSent), _))
            .WillOnce(Invoke([=](uint8_t, span<const uint8_t>, span<uint8_t> outData) {
                std::copy(data.begin(), data.end(), outData.begin());
                return I2CResult::OK;
            }));
        EXPECT_CALL(os, Sleep(_)).Times(0);

        Buffer<200> buffer;
        Writer w(buffer);
        w.WriteByte(0xFF);
        w.WriteByte(0x00);
        w.WriteByte(0x10);
        w.WriteWordLE(0);
        w.WriteArray(dataSent);

        telecommand.Handle(transmitter, w.Capture());
    }
}
