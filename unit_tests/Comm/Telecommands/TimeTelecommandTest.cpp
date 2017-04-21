#include <algorithm>
#include <array>
#include <cmath>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "base/reader.h"
#include "base/writer.h"
#include "mock/time.hpp"
#include "mock/comm.hpp"
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

MATCHER_P(StrSpan, innerMatcher, "")
{
    const char* s = reinterpret_cast<const char*>(arg.data());

    return Matches(innerMatcher)(s);
}

TEST_F(TimeTelecommandTest, ShouldReturnStatusInReadOnlyMode)
{
	EXPECT_CALL(_time, SetCurrentTime(_)).Times(0);
    EXPECT_CALL(_time, SetCurrentTime(IsDownlinkFrame(Eq(static_cast<DownlinkAPID>(0x11)), Eq(1U), Each(Eq(2)))));

    Buffer<3 * DownlinkFrame::MaxPayloadSize> file;
    std::fill_n(file.begin(), DownlinkFrame::MaxPayloadSize, 1);
    std::fill_n(file.begin() + DownlinkFrame::MaxPayloadSize, DownlinkFrame::MaxPayloadSize, 2);
    std::fill_n(file.begin() + 2 * DownlinkFrame::MaxPayloadSize, DownlinkFrame::MaxPayloadSize, 3);

    this->_fs.AddFile("/a/file", file);

    Buffer<200> buffer;
    Writer w(buffer);
    w.WriteByte(0x00); // read time data command

    _telecommand.Handle(_transmitFrame, w.Capture());
}

TEST_F(TimeTelecommandTest, ShouldTransferRequestedPartsOfFile)
{
    EXPECT_CALL(_transmitFrame, SendFrame(IsDownlinkFrame(Eq(static_cast<DownlinkAPID>(0x11)), Eq(0U), Each(Eq(1)))));
    EXPECT_CALL(_transmitFrame, SendFrame(IsDownlinkFrame(Eq(static_cast<DownlinkAPID>(0x11)), Eq(1U), Each(Eq(2)))));

    Buffer<3 * DownlinkFrame::MaxPayloadSize> file;
    std::fill_n(file.begin(), DownlinkFrame::MaxPayloadSize, 1);
    std::fill_n(file.begin() + DownlinkFrame::MaxPayloadSize, DownlinkFrame::MaxPayloadSize, 2);
    std::fill_n(file.begin() + 2 * DownlinkFrame::MaxPayloadSize, DownlinkFrame::MaxPayloadSize, 3);

    this->_fs.AddFile("/a/file", file);

    Buffer<200> buffer;
    Writer w(buffer);
    w.WriteByte(0x11);
    const char path[] = "/a/file";
    w.WriteByte(strlen(path));
    w.WriteArray(gsl::span<const uint8_t>(reinterpret_cast<const uint8_t*>(path), strlen(path)));
    w.WriteByte(0);
    w.WriteDoubleWordLE(0x1);
    w.WriteDoubleWordLE(0x0);

    _telecommand.Handle(_transmitFrame, w.Capture());
}

TEST_F(TimeTelecommandTest, ShouldSendSmallerPartThanMaximumIfNoEnoughDataLength)
{
    EXPECT_CALL(_transmitFrame, SendFrame(IsDownlinkFrame(_, _, SpanOfSize(20))));

    Buffer<20> file;
    std::fill(file.begin(), file.end(), 1);

    this->_fs.AddFile("/a/file", file);

    Buffer<200> buffer;
    Writer w(buffer);
    w.WriteByte(0x11);
    const char path[] = "/a/file";
    w.WriteByte(strlen(path));
    w.WriteArray(gsl::span<const uint8_t>(reinterpret_cast<const uint8_t*>(path), strlen(path)));
    w.WriteByte(0);
    w.WriteDoubleWordLE(0x0);

    _telecommand.Handle(_transmitFrame, w.Capture());
}

TEST_F(TimeTelecommandTest, ShouldNotSendFrameForSequenceNumberBeyondFile)
{
    EXPECT_CALL(_transmitFrame, SendFrame(_)).Times(0);

    Buffer<20> file;
    std::fill(file.begin(), file.end(), 1);

    this->_fs.AddFile("/a/file", file);

    Buffer<200> buffer;
    Writer w(buffer);
    w.WriteByte(0x11);
    const char path[] = "/a/file";
    w.WriteByte(strlen(path));
    w.WriteArray(gsl::span<const uint8_t>(reinterpret_cast<const uint8_t*>(path), strlen(path)));
    w.WriteByte(0);
    w.WriteDoubleWordLE(0x20);

    _telecommand.Handle(_transmitFrame, w.Capture());
}

TEST_F(TimeTelecommandTest, ShouldDownloadLastPartOfMultipartFile)
{
    EXPECT_CALL(_transmitFrame, SendFrame(IsDownlinkFrame(_, _, AllOf(SpanOfSize(20), Each(Eq(4))))));

    Buffer<DownlinkFrame::MaxPayloadSize * 3 + 20> file;
    std::fill_n(file.begin(), DownlinkFrame::MaxPayloadSize, 1);
    std::fill_n(file.begin() + DownlinkFrame::MaxPayloadSize, DownlinkFrame::MaxPayloadSize, 2);
    std::fill_n(file.begin() + 2 * DownlinkFrame::MaxPayloadSize, DownlinkFrame::MaxPayloadSize, 3);
    std::fill_n(file.begin() + 3 * DownlinkFrame::MaxPayloadSize, 20, 4);

    this->_fs.AddFile("/a/file", file);

    Buffer<200> buffer;
    Writer w(buffer);
    w.WriteByte(0x11);
    const char path[] = "/a/file";
    w.WriteByte(strlen(path));
    w.WriteArray(gsl::span<const uint8_t>(reinterpret_cast<const uint8_t*>(path), strlen(path)));
    w.WriteByte(0);
    w.WriteDoubleWordLE(0x3);

    _telecommand.Handle(_transmitFrame, w.Capture());
}

TEST_F(TimeTelecommandTest, ShouldSendErrorFrameWhenFileNotFound)
{
    EXPECT_CALL(_transmitFrame, SendFrame(IsDownlinkFrame(Eq(DownlinkAPID::FileNotFound), Eq(0U), StrSpan(StrEq("/a/file")))));

    Buffer<200> buffer;
    Writer w(buffer);
    w.WriteByte(0x11);
    const char path[] = "/a/file";
    w.WriteByte(strlen(path));
    w.WriteArray(gsl::span<const uint8_t>(reinterpret_cast<const uint8_t*>(path), strlen(path)));
    w.WriteByte(0);
    w.WriteDoubleWordLE(0x3);

    _telecommand.Handle(_transmitFrame, w.Capture());
}
