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

using namespace services::fs;

class FileTest : public Test
{
  protected:
    StrictMock<FsMock> _fs;
};

TEST_F(FileTest, ShouldOpenAndCloseFileAutomatically)
{
    EXPECT_CALL(this->_fs, Open("/file", FileOpen::Existing, FileAccess::ReadOnly)).WillOnce(Return(MakeOpenedFile(1)));
    EXPECT_CALL(this->_fs, Close(1));

    File f(this->_fs, "/file", FileOpen::Existing, FileAccess::ReadOnly);

    ASSERT_THAT(static_cast<bool>(f), Eq(true));
}

TEST_F(FileTest, ShouldNotCloseFileIfOpenFailed)
{
    EXPECT_CALL(this->_fs, Open("/file", _, _)).WillOnce(Return(MakeOpenedFile(OSResult::InvalidOperation)));
    EXPECT_CALL(this->_fs, Close(1)).Times(0);

    File f(this->_fs, "/file", FileOpen::Existing, FileAccess::ReadOnly);
    ASSERT_THAT(static_cast<bool>(f), Eq(false));

    UNREFERENCED_PARAMETER(f);
}

TEST_F(FileTest, ShouldReadFromFile)
{
    std::array<uint8_t, 2> data{1, 2};

    EXPECT_CALL(this->_fs, Open("/file", _, _)).WillOnce(Return(MakeOpenedFile(1)));

    EXPECT_CALL(this->_fs, Read(1, SpanOfSize(2))).WillOnce(Invoke([&data](FileHandle, span<uint8_t> buffer) {
        std::copy(data.begin(), data.end(), buffer.begin());
        return MakeFSIOResult(buffer);
    }));

    EXPECT_CALL(this->_fs, Close(1));

    File f(this->_fs, "/file", FileOpen::Existing, FileAccess::ReadOnly);

    std::array<uint8_t, 2> buffer;

    auto r = f.Read(buffer);

    ASSERT_THAT(r.Result.size(), Eq(2));
    ASSERT_THAT(r.Status, Eq(OSResult::Success));
}

TEST_F(FileTest, ShouldWriteToFile)
{
    std::array<const uint8_t, 2> data{1, 2};

    EXPECT_CALL(this->_fs, Open("/file", _, _)).WillOnce(Return(MakeOpenedFile(1)));

    EXPECT_CALL(this->_fs, Write(1, SpanOfSize(2))).WillOnce(Invoke([&data](FileHandle, span<const uint8_t> buffer) {
        EXPECT_THAT(buffer, ElementsAre(1, 2));
        return MakeFSIOResult(buffer);
    }));

    EXPECT_CALL(this->_fs, Close(1));

    File f(this->_fs, "/file", FileOpen::CreateNew, FileAccess::WriteOnly);

    auto r = f.Write(data);

    ASSERT_THAT(r.Result.size(), Eq(2));
    ASSERT_THAT(r.Status, Eq(OSResult::Success));
}

TEST_F(FileTest, ShouldTruncateFile)
{
    EXPECT_CALL(this->_fs, Open("/file", _, _)).WillOnce(Return(MakeOpenedFile(1)));

    EXPECT_CALL(this->_fs, TruncateFile(1, 10)).WillOnce(Return(OSResult::Success));

    EXPECT_CALL(this->_fs, Close(1));

    File f(this->_fs, "/file", FileOpen::CreateNew, FileAccess::WriteOnly);

    auto r = f.Truncate(10);

    ASSERT_THAT(r, Eq(OSResult::Success));
}
