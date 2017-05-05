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
using std::copy;
using std::uint8_t;
using testing::_;
using testing::Invoke;
using testing::Eq;
using testing::Each;
using testing::ElementsAreArray;
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
namespace
{
    template <std::size_t Size> using Buffer = std::array<uint8_t, Size>;

    class DownloadFileTelecommandTest : public testing::Test
    {
      protected:
        template <std::size_t Size> void SendRequest(uint8_t correlationId, const std::string& path, const std::array<uint16_t, Size> seqs)
        {
            Buffer<200> buffer;
            Writer w(buffer);
            w.WriteByte(correlationId);
            w.WriteByte(path.length());
            w.WriteArray(gsl::span<const uint8_t>(reinterpret_cast<const uint8_t*>(path.data()), path.length()));
            w.WriteByte(0);

            for (auto& seq : seqs)
            {
                w.WriteDoubleWordLE(seq);
            }

            _telecommand.Handle(_transmitter, w.Capture());
        }

        testing::NiceMock<TransmitterMock> _transmitter;
        testing::NiceMock<FsMock> _fs;

        obc::telecommands::DownloadFileTelecommand _telecommand{_fs};
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

    TEST_F(DownloadFileTelecommandTest, ShouldTransferRequestedPartsOfFile)
    {
        const std::string path{"/a/file"};

        std::array<uint8_t, DownlinkFrame::MaxPayloadSize> expectedPayload1;
        std::array<uint8_t, DownlinkFrame::MaxPayloadSize> expectedPayload2;

        expectedPayload1.fill(1);
        expectedPayload2.fill(2);

        expectedPayload1[0] = 0xFF;
        expectedPayload1[1] = static_cast<uint8_t>(DownloadFileTelecommand::ErrorCode::Success);

        expectedPayload2[0] = 0xFF;
        expectedPayload2[1] = static_cast<uint8_t>(DownloadFileTelecommand::ErrorCode::Success);

        EXPECT_CALL(_transmitter, SendFrame(IsDownlinkFrame(Eq(DownlinkAPID::Operation), Eq(0U), ElementsAreArray(expectedPayload1))));
        EXPECT_CALL(_transmitter, SendFrame(IsDownlinkFrame(Eq(DownlinkAPID::Operation), Eq(1U), ElementsAreArray(expectedPayload2))));

        constexpr uint8_t maxFileDataSize = DownlinkFrame::MaxPayloadSize - 2;
        Buffer<3 * maxFileDataSize> file;
        std::fill_n(file.begin(), maxFileDataSize, 1);
        std::fill_n(file.begin() + maxFileDataSize, maxFileDataSize, 2);
        std::fill_n(file.begin() + 2 * maxFileDataSize, maxFileDataSize, 3);

        this->_fs.AddFile(path.c_str(), file);

        this->SendRequest(0xFF, path, std::array<uint16_t, 2>{0x1, 0x0});
    }

    TEST_F(DownloadFileTelecommandTest, ShouldSendSmallerPartThanMaximumIfNoEnoughDataLength)
    {
        const std::string path{"/a/file"};

        EXPECT_CALL(_transmitter, SendFrame(IsDownlinkFrame(_, _, SpanOfSize(22))));

        Buffer<20> file;
        std::fill(file.begin(), file.end(), 1);

        this->_fs.AddFile(path.c_str(), file);

        this->SendRequest(0xFF, path, std::array<uint16_t, 1>{0x0});
    }

    TEST_F(DownloadFileTelecommandTest, ShouldNotSendFrameForSequenceNumberBeyondFile)
    {
        const std::string path{"/a/file"};

        EXPECT_CALL(_transmitter, SendFrame(_)).Times(0);

        Buffer<20> file;
        std::fill(file.begin(), file.end(), 1);

        this->_fs.AddFile(path.c_str(), file);

        this->SendRequest(0xFF, path, std::array<uint16_t, 1>{0x20});
    }

    TEST_F(DownloadFileTelecommandTest, ShouldDownloadLastPartOfMultipartFile)
    {
        const std::string path{"/a/file"};

        std::array<uint8_t, 22> expectedPayload;

        expectedPayload.fill(4);

        expectedPayload[0] = 0xFF;
        expectedPayload[1] = static_cast<uint8_t>(DownloadFileTelecommand::ErrorCode::Success);

        EXPECT_CALL(_transmitter, SendFrame(IsDownlinkFrame(_, _, ElementsAreArray(expectedPayload))));

        constexpr uint8_t maxFileDataSize = DownlinkFrame::MaxPayloadSize - 2;
        Buffer<maxFileDataSize * 3 + 20> file;
        std::fill_n(file.begin(), maxFileDataSize, 1);
        std::fill_n(file.begin() + maxFileDataSize, maxFileDataSize, 2);
        std::fill_n(file.begin() + 2 * maxFileDataSize, maxFileDataSize, 3);
        std::fill_n(file.begin() + 3 * maxFileDataSize, 20, 4);

        this->_fs.AddFile(path.c_str(), file);

        this->SendRequest(0xFF, path, std::array<uint16_t, 1>{0x3});
    }

    TEST_F(DownloadFileTelecommandTest, ShouldSendErrorFrameWhenFileNotFound)
    {
        const std::string path{"/a/file"};

        std::array<uint8_t, 9> expectedPayload;

        expectedPayload[0] = 0xFF;
        expectedPayload[1] = static_cast<uint8_t>(DownloadFileTelecommand::ErrorCode::FileNotFound);
        std::copy(path.begin(), path.end(), expectedPayload.begin() + 2);

        EXPECT_CALL(_transmitter, SendFrame(IsDownlinkFrame(Eq(DownlinkAPID::Operation), Eq(0U), ElementsAreArray(expectedPayload))));

        this->SendRequest(0xFF, path, std::array<uint16_t, 1>{0x3});
    }
}
