#include <array>
#include <string>
#include <tuple>
#include <vector>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "base/reader.h"
#include "mock/FsMock.hpp"
#include "mock/comm.hpp"
#include "obc/telecommands/file_system.hpp"
#include "telecommunication/uplink.h"

using std::array;
using std::uint8_t;
using testing::Eq;
using testing::StrEq;
using testing::_;
using testing::Invoke;
using testing::Contains;
using testing::Pair;
using testing::A;
using testing::Return;
using testing::ElementsAre;
using telecommunication::downlink::DownlinkAPID;
using telecommunication::downlink::DownlinkFrame;
using obc::telecommands::ListFilesTelecommand;
using std::make_pair;
using std::string;
using std::get;
using namespace std::string_literals;

template <std::size_t Size> using BufferArray = array<uint8_t, Size>;
using ResultVector = std::vector<std::pair<std::string, std::size_t>>;

namespace
{
    class ListFilesTelecommandTest : public testing::Test
    {
      protected:
        void ReceiveTo(ResultVector& container);

        testing::NiceMock<FsMock> _fs;
        testing::NiceMock<TransmitterMock> _transmitter;

        ListFilesTelecommand _telecommand{_fs};
    };

    void ListFilesTelecommandTest::ReceiveTo(ResultVector& container)
    {
        ON_CALL(this->_transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::Operation, _, _)))
            .WillByDefault(Invoke([&container](gsl::span<const std::uint8_t> frame) {
                Reader r(frame);
                r.Skip(DownlinkFrame::HeaderSize);
                r.ReadByte();

                while (r.Status() && r.RemainingSize() > 0)
                {
                    string fileName = "";

                    uint8_t b;
                    while ((b = r.ReadByte()) != 0)
                    {
                        fileName += static_cast<char>(b);
                    }

                    auto size = r.ReadDoubleWordLE();

                    container.push_back(make_pair(fileName, size));
                }

                return true;
            }));
    }

    TEST_F(ListFilesTelecommandTest, ShouldListAllFiles)
    {
        BufferArray<1> file;

        this->_fs.AddFile("/a/file1", file);
        this->_fs.AddFile("/a/file2", file);
        this->_fs.AddFile("/a/file3", file);

        ResultVector result;
        ReceiveTo(result);

        BufferArray<20> args{0x11, '/', 'a', 0};

        this->_telecommand.Handle(this->_transmitter, args);

        ASSERT_THAT(result.size(), Eq(3U));

        ASSERT_THAT(get<0>(result[0]), StrEq("file1"));
        ASSERT_THAT(get<1>(result[0]), Eq(1U));

        ASSERT_THAT(get<0>(result[1]), StrEq("file2"));
        ASSERT_THAT(get<1>(result[1]), Eq(1U));

        ASSERT_THAT(get<0>(result[2]), StrEq("file3"));
        ASSERT_THAT(get<1>(result[2]), Eq(1U));
    }

    TEST_F(ListFilesTelecommandTest, ShouldListAllFilesInGivenDirectory)
    {
        BufferArray<10> file;

        this->_fs.AddFile("/b/file1", file);
        this->_fs.AddFile("/a/file2", file);
        this->_fs.AddFile("/c/file3", file);

        ResultVector result;
        ReceiveTo(result);

        BufferArray<20> args{0x11, '/', 'a', 0};

        this->_telecommand.Handle(this->_transmitter, args);

        ASSERT_THAT(result.size(), Eq(1U));

        ASSERT_THAT(get<0>(result[0]), StrEq("file2"));
        ASSERT_THAT(get<1>(result[0]), Eq(10U));
    }

    TEST_F(ListFilesTelecommandTest, ListOfFilesSpanningMoreThanOneFrame)
    {
        BufferArray<1> file;

        this->_fs.AddFile("/a/very_long_name_of_first_file_in_folder", file);
        this->_fs.AddFile("/a/very_long_name_of_second_file_in_folder", file);
        this->_fs.AddFile("/a/very_long_name_of_third_file_in_folder", file);
        this->_fs.AddFile("/a/very_long_name_of_fourth_file_in_folder", file);
        this->_fs.AddFile("/a/very_long_name_of_fifth_file_in_folder", file);
        this->_fs.AddFile("/a/very_long_name_of_sixth_file_in_folder", file);

        ResultVector result;
        ReceiveTo(result);

        BufferArray<20> args{0x11, '/', 'a', 0};

        this->_telecommand.Handle(this->_transmitter, args);

        ASSERT_THAT(result.size(), Eq(6U));

        ASSERT_THAT(result, Contains(Pair("very_long_name_of_first_file_in_folder"s, A<std::size_t>())));
        ASSERT_THAT(result, Contains(Pair("very_long_name_of_second_file_in_folder"s, A<std::size_t>())));
        ASSERT_THAT(result, Contains(Pair("very_long_name_of_third_file_in_folder"s, A<std::size_t>())));
        ASSERT_THAT(result, Contains(Pair("very_long_name_of_fourth_file_in_folder"s, A<std::size_t>())));
        ASSERT_THAT(result, Contains(Pair("very_long_name_of_fifth_file_in_folder"s, A<std::size_t>())));
        ASSERT_THAT(result, Contains(Pair("very_long_name_of_sixth_file_in_folder"s, A<std::size_t>())));
    }

    TEST_F(ListFilesTelecommandTest, RespondWithErrorFrameIfUnableToOpenDirectory)
    {
        ON_CALL(this->_fs, OpenDirectory(_)).WillByDefault(Return(services::fs::DirectoryOpenResult(OSResult::DeviceNotFound, nullptr)));

        EXPECT_CALL(this->_transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::Operation, 0, ElementsAre(0x11, 19))));

        BufferArray<20> args{0x11, '/', 'a', 0};

        this->_telecommand.Handle(this->_transmitter, args);
    }
}
