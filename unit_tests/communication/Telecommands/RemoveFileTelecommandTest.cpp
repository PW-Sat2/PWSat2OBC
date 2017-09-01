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
        testing::NiceMock<TransmitterMock> _transmitter;
        testing::NiceMock<FsMock> _fs;

        obc::telecommands::RemoveFileTelecommand _telecommand{_fs};
    };

    TEST_F(RemoveFileTelecommandTest, ShouldRemoveFile)
    {
        const std::string path{"/a/file"};

        std::array<uint8_t, 9> expectedPayload;

        expectedPayload[0] = 0xFF;
        expectedPayload[1] = 0x00;
        std::copy(path.begin(), path.end(), expectedPayload.begin() + 2);

        EXPECT_CALL(_transmitter, SendFrame(IsDownlinkFrame(Eq(DownlinkAPID::FileRemove), Eq(0U), ElementsAreArray(expectedPayload))));

        Buffer<1> file;
        this->_fs.AddFile(path.c_str(), file);

        Buffer<200> buffer;
        Writer w(buffer);
        w.WriteByte(0xFF);
        w.WriteByte(path.length());
        w.WriteArray(gsl::span<const uint8_t>(reinterpret_cast<const uint8_t*>(path.data()), path.length()));
        w.WriteByte(0);

        _telecommand.Handle(_transmitter, w.Capture());
    }

    TEST_F(RemoveFileTelecommandTest, ShouldSendErrorFrameWhenFileNotFound)
    {
        const std::string path{"/a/file"};

        std::array<uint8_t, 9> expectedPayload;

        expectedPayload[0] = 0xFF;
        expectedPayload[1] = static_cast<uint8_t>(OSResult::NotFound);
        std::copy(path.begin(), path.end(), expectedPayload.begin() + 2);

        EXPECT_CALL(_transmitter, SendFrame(IsDownlinkFrame(Eq(DownlinkAPID::FileRemove), Eq(0U), ElementsAreArray(expectedPayload))));

        ON_CALL(this->_fs, Unlink(_)).WillByDefault(Return(OSResult::NotFound));

        Buffer<200> buffer;
        Writer w(buffer);
        w.WriteByte(0xFF);
        w.WriteByte(path.length());
        w.WriteArray(gsl::span<const uint8_t>(reinterpret_cast<const uint8_t*>(path.data()), path.length()));
        w.WriteByte(0);

        _telecommand.Handle(_transmitter, w.Capture());
    }

    TEST_F(RemoveFileTelecommandTest, ShouldSendErrorFrameWhenPathNotNullTerminated)
    {
        const std::string path{'\x64', '\x64', '\x64'};

        std::array<uint8_t, 3> expectedPayload{0xFF, static_cast<uint8_t>(OSResult::InvalidArgument), 0x00};

        EXPECT_CALL(_transmitter, SendFrame(IsDownlinkFrame(Eq(DownlinkAPID::FileRemove), Eq(0U), ElementsAreArray(expectedPayload))));

        Buffer<200> buffer;
        Writer w(buffer);
        w.WriteByte(0xFF);
        w.WriteByte(2);
        w.WriteArray(gsl::span<const uint8_t>(reinterpret_cast<const uint8_t*>(path.data()), path.length()));
        w.WriteByte(0);
        _telecommand.Handle(_transmitter, w.Capture());
    }

    TEST_F(RemoveFileTelecommandTest, ShouldSendErrorFrameWhenDataTooShort)
    {
        const std::string path{'\x64', '\x64', '\x64'};

        std::array<uint8_t, 3> expectedPayload{0xFF, static_cast<uint8_t>(OSResult::InvalidArgument), 0x00};

        EXPECT_CALL(_transmitter, SendFrame(IsDownlinkFrame(Eq(DownlinkAPID::FileRemove), Eq(0U), ElementsAreArray(expectedPayload))));

        Buffer<200> buffer;
        Writer w(buffer);
        w.WriteByte(0xFF);
        w.WriteByte(0xFF);
        _telecommand.Handle(_transmitter, w.Capture());
    }
}
