#include <array>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "base/reader.h"
#include "base/writer.h"
#include "mock/comm.hpp"
#include "obc/telecommands/comm.hpp"
#include "telecommunication/downlink.h"
#include "telecommunication/telecommand_handling.h"

using std::array;
using std::uint8_t;
using testing::_;
using testing::Eq;
using testing::Return;
using testing::ElementsAreArray;

using telecommunication::downlink::DownlinkFrame;
using telecommunication::downlink::DownlinkAPID;

using devices::comm::Bitrate;

namespace
{
    template <std::size_t Size> using Buffer = std::array<uint8_t, Size>;

    class SetBitrateTelecommandTest : public testing::Test
    {
      protected:
        testing::NiceMock<TransmitterMock> transmitter;

        obc::telecommands::SetBitrateTelecommand telecommand;
    };

    TEST_F(SetBitrateTelecommandTest, ShouldSetBitrate)
    {
        EXPECT_CALL(transmitter, SetTransmitterBitRate(Bitrate::Comm9600bps)).WillOnce(Return(true));

        Buffer<200> buffer;
        Writer w(buffer);
        w.WriteByte(0xFF);
        w.WriteWordLE(0x8);

        telecommand.Handle(transmitter, w.Capture());
    }

    TEST_F(SetBitrateTelecommandTest, ShouldSendResponse)
    {
        ON_CALL(transmitter, SetTransmitterBitRate(_)).WillByDefault(Return(true));
        EXPECT_CALL(transmitter, SendFrame(IsDownlinkFrame(Eq(DownlinkAPID::SetBitrate), Eq(0U), ElementsAreArray({0xFF, 0}))));

        Buffer<200> buffer;
        Writer w(buffer);
        w.WriteByte(0xFF);
        w.WriteWordLE(0x8);

        telecommand.Handle(transmitter, w.Capture());
    }
}
