#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "base/os.h"
#include "base/writer.h"
#include "mock/comm.hpp"
#include "obc/telecommands/comm.hpp"
#include "telecommunication/downlink.h"
#include "telecommunication/telecommand_handling.h"
#include "utils.hpp"

using testing::_;

namespace
{
    template <std::size_t Size> using Buffer = std::array<uint8_t, Size>;

    class ResetTransmitterTelecommandTest : public testing::Test
    {
      protected:
        ResetTransmitterTelecommandTest();
        testing::NiceMock<TransmitterMock> _transmitter;

        obc::telecommands::ResetTransmitterTelecommand _telecommand;
    };

    ResetTransmitterTelecommandTest::ResetTransmitterTelecommandTest()
    {
    }

    TEST_F(ResetTransmitterTelecommandTest, ShouldResetTransmitterAndNotSendResponse)
    {
        EXPECT_CALL(_transmitter, SendFrame(_)).Times(0);
        EXPECT_CALL(_transmitter, ResetTransmitter()).Times(1);

        Buffer<200> buffer;
        Writer w(buffer);
        w.WriteByte(0xFF);
        w.WriteByte(0x01);

        _telecommand.Handle(_transmitter, w.Capture());
    }

    TEST_F(ResetTransmitterTelecommandTest, ShouldResetTransmitterOnNoArguments)
    {
        EXPECT_CALL(_transmitter, SendFrame(_)).Times(0);
        EXPECT_CALL(_transmitter, ResetTransmitter()).Times(1);

        _telecommand.Handle(_transmitter, nullptr);
    }
}
