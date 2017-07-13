#include <algorithm>
#include <array>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "I2C/I2CMock.hpp"
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
        testing::NiceMock<TransmitterMock> transmitter;
        testing::NiceMock<I2CBusMock> i2c;

        obc::telecommands::RawI2CTelecommand telecommand{i2c};
    };

    TEST_F(RawI2CTelecommandTest, ShouldSendData)
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
        EXPECT_CALL(i2c, WriteRead(0x01, ElementsAreArray(dataSent), _))
            .WillOnce(Invoke([=](uint8_t, span<const uint8_t>, span<uint8_t> outData) {
                std::copy(data.begin(), data.end(), outData.begin());
                return I2CResult::OK;
            }));

        Buffer<200> buffer;
        Writer w(buffer);
        w.WriteByte(0xFF);
        w.WriteByte(0x01);
        w.WriteArray(dataSent);

        telecommand.Handle(transmitter, w.Capture());
    }
}
