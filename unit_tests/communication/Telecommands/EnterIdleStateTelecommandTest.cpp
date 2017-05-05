#include <algorithm>
#include <array>
#include <cmath>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "base/reader.h"
#include "base/writer.h"
#include "mock/IdleStateControllerMock.hpp"
#include "mock/comm.hpp"
#include "mock/time.hpp"
#include "obc/telecommands/comm.hpp"
#include "telecommunication/downlink.h"
#include "telecommunication/telecommand_handling.h"
#include "utils.hpp"

using std::array;
using std::uint8_t;
using testing::_;
using testing::Invoke;
using testing::Eq;
using testing::Each;
using testing::StrEq;
using testing::Return;
using testing::Matches;
using testing::AllOf;
using gsl::span;

using obc::telecommands::EnterIdleStateTelecommand;
using telecommunication::downlink::DownlinkFrame;
using telecommunication::downlink::DownlinkAPID;
using testing::Eq;
using testing::ElementsAreArray;

namespace
{
    template <std::size_t Size> using Buffer = std::array<uint8_t, Size>;

    class EnterIdleStateTelecommandTest : public testing::Test
    {
      protected:
        testing::NiceMock<TransmitterMock> _transmitter;
        testing::NiceMock<CurrentTimeMock> _currentTime;
        testing::NiceMock<IdleStateControllerMock> _idleStateController;

        obc::telecommands::EnterIdleStateTelecommand _telecommand{_currentTime, _idleStateController};
    };

    MATCHER_P3(IsDownlinkFrame, apidMatcher, seqMatcher, payloadMatcher, "")
    {
        DownlinkAPID apid = static_cast<DownlinkAPID>((arg[0] & 0b11111100) >> 2);
        std::uint32_t seq = ((arg[0] & 0b11) << 16) //
            | (arg[1] << 8)                         //
            | (arg[2]);
        auto payload = arg.subspan(3);

        return Matches(apidMatcher)(apid) //
            && Matches(seqMatcher)(seq)   //
            && Matches(payloadMatcher)(payload);
    }

    TEST_F(EnterIdleStateTelecommandTest, ShouldSetIdleState)
    {
        ON_CALL(_currentTime, GetCurrentTime())
            .WillByDefault(Return(Option<std::chrono::milliseconds>::Some(std::chrono::milliseconds{0})));
        EXPECT_CALL(_idleStateController, EnterTransmitterStateWhenIdle(Eq(std::chrono::milliseconds{1000})));

        Buffer<200> buffer;
        Writer w(buffer);
        w.WriteByte(0xFF);
        w.WriteByte(0x01);

        _telecommand.Handle(_transmitter, w.Capture());
    }

    TEST_F(EnterIdleStateTelecommandTest, ShouldReportSuccessWhenCanSetIdleState)
    {
        ON_CALL(_idleStateController, EnterTransmitterStateWhenIdle(_)).WillByDefault(Return(true));
        EXPECT_CALL(_transmitter, SendFrame(IsDownlinkFrame(Eq(DownlinkAPID::Operation), Eq(0U), ElementsAreArray({0xFF, 0x00}))));

        Buffer<200> buffer;
        Writer w(buffer);
        w.WriteByte(0xFF);
        w.WriteByte(0x01);

        _telecommand.Handle(_transmitter, w.Capture());
    }

    TEST_F(EnterIdleStateTelecommandTest, ShouldReportFailureWhenCannotSetIdleState)
    {
        ON_CALL(_idleStateController, EnterTransmitterStateWhenIdle(_)).WillByDefault(Return(false));
        EXPECT_CALL(_transmitter, SendFrame(IsDownlinkFrame(Eq(DownlinkAPID::Operation), Eq(0U), ElementsAreArray({0xFF, 0xFF}))));

        Buffer<200> buffer;
        Writer w(buffer);
        w.WriteByte(0xFF);
        w.WriteByte(0x01);

        _telecommand.Handle(_transmitter, w.Capture());
    }
}
