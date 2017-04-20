#include <algorithm>
#include <array>
#include <cmath>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "base/reader.h"
#include "base/writer.h"
#include "fs/fs.h"
#include "mock/FsMock.hpp"
#include "mock/comm.hpp"
#include "obc/telecommands/file_system.hpp"
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

using services::fs::File;
using services::fs::FileHandle;
using services::fs::SeekOrigin;
using obc::telecommands::DownloadFileTelecommand;
using telecommunication::downlink::DownlinkFrame;
using telecommunication::downlink::DownlinkAPID;

template <std::size_t Size> using Buffer = std::array<uint8_t, Size>;

class RemoveFileTelecommandTest : public testing::Test
{
  protected:
    testing::NiceMock<TransmitFrameMock> _transmitFrame;
    testing::NiceMock<FsMock> _fs;

    obc::telecommands::RemoveFileTelecommand _telecommand{_fs};
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

TEST_F(RemoveFileTelecommandTest, ShouldRemoveFile)
{
    EXPECT_CALL(_transmitFrame, SendFrame(IsDownlinkFrame(Eq(static_cast<DownlinkAPID>(0x11)), Eq(0U), StrSpan(StrEq("\0/a/file")))));

    Buffer<1> file;
    this->_fs.AddFile("/a/file", file);

    Buffer<200> buffer;
    Writer w(buffer);
    const char path[] = "/a/file";
    w.WriteByte(0x11);
    w.WriteByte(strlen(path));
    w.WriteArray(gsl::span<const uint8_t>(reinterpret_cast<const uint8_t*>(path), strlen(path)));
    w.WriteByte(0);

    _telecommand.Handle(_transmitFrame, w.Capture());
}

TEST_F(RemoveFileTelecommandTest, ShouldSendErrorFrameWhenFileNotFound)
{
    EXPECT_CALL(_transmitFrame, SendFrame(IsDownlinkFrame(Eq(static_cast<DownlinkAPID>(0x11)), Eq(0U), StrSpan(StrEq("\2/a/not_found")))));

    ON_CALL(this->_fs, Unlink(_)).WillByDefault(Return(OSResult::NotFound));

    Buffer<200> buffer;
    Writer w(buffer);
    const char path[] = "/a/not_found";
    w.WriteByte(0x11);
    w.WriteByte(strlen(path));
    w.WriteArray(gsl::span<const uint8_t>(reinterpret_cast<const uint8_t*>(path), strlen(path)));
    w.WriteByte(0);

    _telecommand.Handle(_transmitFrame, w.Capture());
}
