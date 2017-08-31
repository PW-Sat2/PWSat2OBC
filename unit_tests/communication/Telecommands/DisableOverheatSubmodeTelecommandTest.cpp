#include <algorithm>
#include <array>
#include <chrono>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "base/reader.h"
#include "base/writer.h"
#include "mock/comm.hpp"
#include "mock/eps.hpp"
#include "obc/telecommands/eps.hpp"
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
using devices::eps::IEPSDriver;

using gsl::span;

template <std::size_t Size> using Buffer = std::array<std::uint8_t, Size>;

namespace
{
    class DisableOverheatSubmodeTelecommandTest : public testing::Test
    {
      protected:
        testing::NiceMock<TransmitterMock> transmitter;
        testing::NiceMock<EPSDriverMock> epsDriver;

        obc::telecommands::DisableOverheatSubmodeTelecommand telecommand{epsDriver};
    };

    TEST_F(DisableOverheatSubmodeTelecommandTest, ShouldDisableOverheatSubmode)
    {
        EXPECT_CALL(epsDriver, DisableOverheatSubmode(IEPSDriver::Controller::B)).WillOnce(Return(true));

        Buffer<200> buffer;
        Writer w(buffer);
        w.WriteByte(0xFF);
        w.WriteWordLE(0x1);

        telecommand.Handle(transmitter, w.Capture());
    }

    TEST_F(DisableOverheatSubmodeTelecommandTest, ShouldSendResponse)
    {
        ON_CALL(epsDriver, DisableOverheatSubmode(_)).WillByDefault(Return(true));
        EXPECT_CALL(transmitter, SendFrame(IsDownlinkFrame(Eq(DownlinkAPID::DisableOverheatSubmode), Eq(0U), ElementsAreArray({0xFF, 0}))));

        Buffer<200> buffer;
        Writer w(buffer);
        w.WriteByte(0xFF);
        w.WriteWordLE(0x1);

        telecommand.Handle(transmitter, w.Capture());
    }
}
