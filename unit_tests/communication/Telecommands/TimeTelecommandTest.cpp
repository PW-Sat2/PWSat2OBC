#include <algorithm>
#include <array>
#include <cmath>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "base/reader.h"
#include "base/writer.h"
#include "mock/comm.hpp"
#include "mock/time.hpp"
#include "obc/telecommands/time.hpp"
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

using obc::telecommands::TimeTelecommand;
using telecommunication::downlink::DownlinkFrame;
using telecommunication::downlink::DownlinkAPID;

template <std::size_t Size> using Buffer = std::array<uint8_t, Size>;

class TimeTelecommandTest : public testing::Test
{
  protected:
    testing::NiceMock<TransmitFrameMock> _transmitFrame;
    testing::NiceMock<CurrentTimeMock> _time;

    obc::telecommands::TimeTelecommand _telecommand{_time};
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

MATCHER_P4(IsTimeDownlinkPayloadMatcher, optionMatcher, resultMatcher, argumentMatcher, telemetryMatcher, "")
{
    const uint8_t* buffer = arg.data();

    auto option = buffer[0];
    auto result = buffer[1];

    // I want uint64_t, so C magic - get address of next byte, think it is byte array, rethink it is ULL array, get first element.
    uint64_t argument = *(uint64_t*)(uint8_t*)(&(buffer[2]));

    auto payload = &(buffer[10]);

    return Matches(optionMatcher)(option) //
        && Matches(resultMatcher)(result) //
        && Matches(argumentMatcher)(argument) && Matches(telemetryMatcher)(payload);
}

TEST_F(TimeTelecommandTest, ShouldOnlyReturnStatusWhenInReadOnlyMode)
{
    EXPECT_CALL(_time, SetCurrentTime(_)).Times(0);
    EXPECT_CALL(_time, GetCurrentTime()).Times(1);
    EXPECT_CALL(_transmitFrame,
        SendFrame(IsDownlinkFrame(Eq(DownlinkAPID::TimeStatus),
            _,
            IsTimeDownlinkPayloadMatcher(Eq(num(TimeTelecommand::TimeOperations::ReadOnly)), Eq(num(OSResult::Success)), _, _))));

    Buffer<200> buffer;
    Writer w(buffer);
    w.WriteByte(0x00); // read time data command

    _telecommand.Handle(_transmitFrame, w.Capture());
}

TEST_F(TimeTelecommandTest, ShouldSetTimeWhenTimeOptionProvided)
{
    time_t currentTime = time(0);
    uint64_t currentTimeToSend = static_cast<uint64_t>(currentTime);
    auto currentTimeAsMilliseconds = std::chrono::milliseconds(currentTimeToSend);

    EXPECT_CALL(_time, SetCurrentTime(Eq(currentTimeAsMilliseconds))).Times(1);
    EXPECT_CALL(_time, GetCurrentTime()).Times(1);
    EXPECT_CALL(_transmitFrame,
        SendFrame(IsDownlinkFrame(Eq(DownlinkAPID::TimeStatus),
            _,
            IsTimeDownlinkPayloadMatcher(
                Eq(num(TimeTelecommand::TimeOperations::Time)), Eq(num(OSResult::Success)), Eq(currentTimeToSend), _))));

    Buffer<200> buffer;
    Writer w(buffer);
    w.WriteByte(0x01); // write time data command
    w.WriteQuadWordLE(currentTimeToSend);

    _telecommand.Handle(_transmitFrame, w.Capture());
}

TEST_F(TimeTelecommandTest, ShouldSendErrorFrameWhenSetTimeCorrectionOptionProvided)
{
    int16_t correction = 0x1234;

    EXPECT_CALL(_time, SetCurrentTime(_)).Times(0);
    EXPECT_CALL(_time, GetCurrentTime()).Times(1);

    EXPECT_CALL(_transmitFrame,
        SendFrame(IsDownlinkFrame(Eq(DownlinkAPID::TimeStatus),
            _,
            IsTimeDownlinkPayloadMatcher(Eq(num(TimeTelecommand::TimeOperations::TimeCorrection)),
                Eq(num(OSResult::NotImplemented)),
                Eq(static_cast<uint64_t>(correction)),
                _))));

    Buffer<200> buffer;
    Writer w(buffer);
    w.WriteByte(0x02); // write time correction data command
    w.WriteWordLE(correction);

    _telecommand.Handle(_transmitFrame, w.Capture());
}

TEST_F(TimeTelecommandTest, ShouldSendErrorFrameWhenSetRtcTimeCorrectionOptionProvided)
{
    int16_t correction = 0x1234;

    EXPECT_CALL(_time, SetCurrentTime(_)).Times(0);
    EXPECT_CALL(_time, GetCurrentTime()).Times(1);

    EXPECT_CALL(_transmitFrame,
        SendFrame(IsDownlinkFrame(Eq(DownlinkAPID::TimeStatus),
            _,
            IsTimeDownlinkPayloadMatcher(Eq(num(TimeTelecommand::TimeOperations::RTCCorrection)),
                Eq(num(OSResult::NotImplemented)),
                Eq(static_cast<uint64_t>(correction)),
                _))));

    Buffer<200> buffer;
    Writer w(buffer);
    w.WriteByte(0x03); // write rtc time correction data command
    w.WriteWordLE(correction);

    _telecommand.Handle(_transmitFrame, w.Capture());
}

TEST_F(TimeTelecommandTest, ShouldSendErrorFrameWhenNegativeSetTimeCorrectionOptionProvided)
{
    int16_t correction = -1234;

    EXPECT_CALL(_time, SetCurrentTime(_)).Times(0);
    EXPECT_CALL(_time, GetCurrentTime()).Times(1);

    EXPECT_CALL(_transmitFrame,
        SendFrame(IsDownlinkFrame(Eq(DownlinkAPID::TimeStatus),
            _,
            IsTimeDownlinkPayloadMatcher(Eq(num(TimeTelecommand::TimeOperations::TimeCorrection)),
                Eq(num(OSResult::NotImplemented)),
                Eq(static_cast<uint64_t>(correction)),
                _))));

    Buffer<200> buffer;
    Writer w(buffer);
    w.WriteByte(0x02); // write time correction data command
    w.WriteWordLE(correction);

    _telecommand.Handle(_transmitFrame, w.Capture());
}

TEST_F(TimeTelecommandTest, ShouldSendErrorFrameWhenNegativeSetRtcTimeCorrectionOptionProvided)
{
    int16_t correction = -1234;

    EXPECT_CALL(_time, SetCurrentTime(_)).Times(0);
    EXPECT_CALL(_time, GetCurrentTime()).Times(1);

    EXPECT_CALL(_transmitFrame,
        SendFrame(IsDownlinkFrame(Eq(DownlinkAPID::TimeStatus),
            _,
            IsTimeDownlinkPayloadMatcher(Eq(num(TimeTelecommand::TimeOperations::RTCCorrection)),
                Eq(num(OSResult::NotImplemented)),
                Eq(static_cast<uint64_t>(correction)),
                _))));

    Buffer<200> buffer;
    Writer w(buffer);
    w.WriteByte(0x03); // write rtc time correction data command
    w.WriteWordLE(correction);

    _telecommand.Handle(_transmitFrame, w.Capture());
}
