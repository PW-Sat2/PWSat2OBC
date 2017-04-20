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
using testing::Eq;
using testing::ElementsAreArray;
using testing::Return;
using testing::Matches;
using gsl::span;

using obc::telecommands::DownloadFileTelecommand;
using telecommunication::downlink::DownlinkFrame;
using telecommunication::downlink::DownlinkAPID;

template <std::size_t Size> using Buffer = std::array<uint8_t, Size>;

namespace
{
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
        const std::string path{"/a/file"};

        std::array<uint8_t, 9> expectedPayload;

        expectedPayload[0] = 0xFF;
        expectedPayload[1] = 0x00;
        std::copy(path.begin(), path.end(), expectedPayload.begin() + 2);

        EXPECT_CALL(_transmitFrame, SendFrame(IsDownlinkFrame(Eq(DownlinkAPID::Operation), Eq(0U), ElementsAreArray(expectedPayload))));

        Buffer<1> file;
        this->_fs.AddFile(path.c_str(), file);

        Buffer<200> buffer;
        Writer w(buffer);
        w.WriteByte(0xFF);
        w.WriteByte(path.length());
        w.WriteArray(gsl::span<const uint8_t>(reinterpret_cast<const uint8_t*>(path.data()), path.length()));
        w.WriteByte(0);

        _telecommand.Handle(_transmitFrame, w.Capture());
    }

    TEST_F(RemoveFileTelecommandTest, ShouldSendErrorFrameWhenFileNotFound)
    {
        const std::string path{"/a/file"};

        std::array<uint8_t, 9> expectedPayload;

        expectedPayload[0] = 0xFF;
        expectedPayload[1] = static_cast<uint8_t>(OSResult::NotFound);
        std::copy(path.begin(), path.end(), expectedPayload.begin() + 2);

        EXPECT_CALL(_transmitFrame, SendFrame(IsDownlinkFrame(Eq(DownlinkAPID::Operation), Eq(0U), ElementsAreArray(expectedPayload))));

        ON_CALL(this->_fs, Unlink(_)).WillByDefault(Return(OSResult::NotFound));

        Buffer<200> buffer;
        Writer w(buffer);
        w.WriteByte(0xFF);
        w.WriteByte(path.length());
        w.WriteArray(gsl::span<const uint8_t>(reinterpret_cast<const uint8_t*>(path.data()), path.length()));
        w.WriteByte(0);

        _telecommand.Handle(_transmitFrame, w.Capture());
    }
}
