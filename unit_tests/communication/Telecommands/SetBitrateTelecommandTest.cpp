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
using testing::ReturnRef;

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
}
