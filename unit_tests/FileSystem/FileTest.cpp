#include <algorithm>
#include <array>
#include <cstdint>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gsl/span"

#include "fs/fs.h"
#include "mock/FsMock.hpp"
#include "utils.hpp"

using gsl::span;
using testing::_;
using testing::Eq;
using testing::ElementsAre;
using testing::Test;
using testing::StrictMock;
using testing::Return;
using testing::Invoke;

class FileTest : public Test
{
  protected:
    StrictMock<FsMock> _fs;
};

TEST_F(FileTest, ShouldOpenAndCloseFileAutomatically)
{
    EXPECT_CALL(this->_fs, Open("/file", FSFileOpen::Existing, FSFileAccess::ReadOnly)).WillOnce(Return(MakeOpenedFile(1)));
    EXPECT_CALL(this->_fs, Close(1));

    auto f = File::OpenRead(this->_fs, "/file");

    ASSERT_THAT(static_cast<bool>(f), Eq(true));
}

TEST_F(FileTest, ShouldNotCloseFileIfOpenFailed)
{
    EXPECT_CALL(this->_fs, Open("/file", FSFileOpen::Existing, FSFileAccess::ReadOnly))
        .WillOnce(Return(MakeOpenedFile(OSResult::InvalidOperation)));
    EXPECT_CALL(this->_fs, Close(1)).Times(0);

    auto f = File::OpenRead(this->_fs, "/file");
    ASSERT_THAT(static_cast<bool>(f), Eq(false));

    UNREFERENCED_PARAMETER(f);
}

TEST_F(FileTest, ShouldOpenFileForWrite)
{
    EXPECT_CALL(this->_fs, Open("/file", FSFileOpen::CreateAlways, FSFileAccess::ReadWrite)).WillOnce(Return(MakeOpenedFile(1)));
    EXPECT_CALL(this->_fs, Close(1));

    auto f = File::OpenWrite(this->_fs, "/file", FSFileOpen::CreateAlways, FSFileAccess::ReadWrite);

    UNREFERENCED_PARAMETER(f);
}

TEST_F(FileTest, ShouldReadFromFile)
{
    std::array<uint8_t, 2> data{1, 2};

    EXPECT_CALL(this->_fs, Open("/file", _, _)).WillOnce(Return(MakeOpenedFile(1)));

    EXPECT_CALL(this->_fs, Read(1, SpanOfSize(2))).WillOnce(Invoke([&data](FSFileHandle, span<uint8_t> buffer) {
        std::copy(data.begin(), data.end(), buffer.begin());
        return MakeFSIOResult(2);
    }));

    EXPECT_CALL(this->_fs, Close(1));

    auto f = File::OpenRead(this->_fs, "/file");

    std::array<uint8_t, 2> buffer;

    auto r = f.Read(buffer);

    ASSERT_THAT(r.BytesTransferred, Eq(2));
    ASSERT_THAT(r.Status, Eq(OSResult::Success));
}

TEST_F(FileTest, ShouldWriteToFile)
{
    std::array<const uint8_t, 2> data{1, 2};

    EXPECT_CALL(this->_fs, Open("/file", _, _)).WillOnce(Return(MakeOpenedFile(1)));

    EXPECT_CALL(this->_fs, Write(1, SpanOfSize(2))).WillOnce(Invoke([&data](FSFileHandle, span<const uint8_t> buffer) {
        EXPECT_THAT(buffer, ElementsAre(1, 2));
        return MakeFSIOResult(2);
    }));

    EXPECT_CALL(this->_fs, Close(1));

    auto f = File::OpenWrite(this->_fs, "/file");

    auto r = f.Write(data);

    ASSERT_THAT(r.BytesTransferred, Eq(2));
    ASSERT_THAT(r.Status, Eq(OSResult::Success));
}

TEST_F(FileTest, ShouldTruncateFile)
{
    EXPECT_CALL(this->_fs, Open("/file", _, _)).WillOnce(Return(MakeOpenedFile(1)));

    EXPECT_CALL(this->_fs, TruncateFile(1, 10)).WillOnce(Return(OSResult::Success));

    EXPECT_CALL(this->_fs, Close(1));

    auto f = File::OpenWrite(this->_fs, "/file");

    auto r = f.Truncate(10);

    ASSERT_THAT(r, Eq(OSResult::Success));
}
