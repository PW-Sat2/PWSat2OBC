#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "OsMock.hpp"
#include "mock/FsMock.hpp"
#include "os/os.hpp"

#include "time/timer.h"

using testing::_;
using testing::Eq;
using testing::Ne;
using testing::Return;
using testing::Invoke;

class TimerPersistanceTest : public testing::Test
{
  protected:
    TimerPersistanceTest();
    TimeSpan GetCurrentTime();

    TimeProvider provider;
    testing::NiceMock<FsMock> fs;
    testing::NiceMock<OSMock> os;
    OSReset osGuard;
    FsMockReset fsGuard;
};

TimeSpan TimerPersistanceTest::GetCurrentTime()
{
    TimeSpan span = 0;
    EXPECT_TRUE(TimeGetCurrentTime(&provider, &span));
    return span;
}

static void TimePassedProxy(void* /*context*/, TimePoint /*currentTime*/)
{
}

TimerPersistanceTest::TimerPersistanceTest()
{
    this->osGuard = InstallProxy(&os);
    this->fsGuard = InstallFileSystemMock(fs);
    EXPECT_CALL(os, CreateBinarySemaphore()).WillOnce(Return(reinterpret_cast<void*>(1))).WillOnce(Return(reinterpret_cast<void*>(2)));
    ON_CALL(os, TakeSemaphore(_, _)).WillByDefault(Return(OSResultSuccess));
    ON_CALL(os, GiveSemaphore(_)).WillByDefault(Return(OSResultSuccess));
    ON_CALL(fs, GetLastError()).WillByDefault(Return(0));
}

TEST_F(TimerPersistanceTest, TestReadingStateNoState)
{
    EXPECT_CALL(fs, Open(_, _, _)).WillRepeatedly(Return(-1));
    EXPECT_TRUE(TimeInitialize(&provider, TimePassedProxy, nullptr, &MockedFileSystem));
    ASSERT_THAT(GetCurrentTime(), Eq(0u));
}

TEST_F(TimerPersistanceTest, TestReadingStateEmptyFiles)
{
    EXPECT_CALL(fs, Open(_, _, _)).WillRepeatedly(Return(1));
    EXPECT_CALL(fs, Read(_, _, _)).WillRepeatedly(Return(-1));
    EXPECT_CALL(fs, Close(_)).WillRepeatedly(Return(0));
    EXPECT_TRUE(TimeInitialize(&provider, TimePassedProxy, nullptr, &MockedFileSystem));
    ASSERT_THAT(GetCurrentTime(), Eq(0u));
}

TEST_F(TimerPersistanceTest, TestReadingFiles)
{
    EXPECT_CALL(fs, Open(_, _, _)).WillRepeatedly(Return(10));
    EXPECT_CALL(fs, Close(_)).WillRepeatedly(Return(0));
    EXPECT_CALL(fs, Read(_, _, _))
        .WillRepeatedly(Invoke([](FSFileHandle /*file*/, void* buffer, unsigned int size) {
            EXPECT_THAT(buffer, Ne(nullptr));
            if (buffer == nullptr)
            {
                return -1;
            }
            else
            {
                memset(buffer, 0x11, size);
                return static_cast<int>(size);
            }
        }));
    EXPECT_TRUE(TimeInitialize(&provider, TimePassedProxy, nullptr, &MockedFileSystem));
    ASSERT_THAT(GetCurrentTime(), Eq(0x1111111111111111ull));
}

TEST_F(TimerPersistanceTest, TestReadingSingleNonEmptyFile)
{
    EXPECT_CALL(fs, Open(_, _, _)).WillOnce(Return(1)).WillRepeatedly(Return(-1));
    EXPECT_CALL(fs, Close(_)).WillRepeatedly(Return(0));
    EXPECT_CALL(fs, Read(_, _, _))
        .WillOnce(Invoke([](FSFileHandle /*file*/, void* buffer, unsigned int size) {
            memset(buffer, 0x11, size);
            return size;
        }))
        .WillRepeatedly(Return(-1));
    EXPECT_TRUE(TimeInitialize(&provider, TimePassedProxy, nullptr, &MockedFileSystem));
    ASSERT_THAT(GetCurrentTime(), Eq(0u));
}

TEST_F(TimerPersistanceTest, TestReadingTwoNonEmptyFiles)
{
    EXPECT_CALL(fs, Open(_, _, _)).WillRepeatedly(Return(1));
    EXPECT_CALL(fs, Close(_)).WillRepeatedly(Return(0));
    EXPECT_CALL(fs, Read(_, _, _))
        .WillOnce(Return(0))
        .WillRepeatedly(Invoke([](FSFileHandle /*file*/, void* buffer, unsigned int size) {
            memset(buffer, 0x11, size);
            return static_cast<int>(size);
        }));
    EXPECT_TRUE(TimeInitialize(&provider, TimePassedProxy, nullptr, &MockedFileSystem));
    ASSERT_THAT(GetCurrentTime(), Eq(0x1111111111111111ull));
}

TEST_F(TimerPersistanceTest, TestReadingTwoExistingEmptyFiles)
{
    EXPECT_CALL(fs, Open(_, _, _)).WillOnce(Return(-1)).WillRepeatedly(Return(1));
    EXPECT_CALL(fs, Close(_)).WillRepeatedly(Return(0));
    EXPECT_CALL(fs, Read(_, _, _))
        .WillRepeatedly(Invoke([](FSFileHandle /*file*/, void* buffer, unsigned int size) {
            memset(buffer, 0x11, size);
            return static_cast<int>(size);
        }));
    EXPECT_TRUE(TimeInitialize(&provider, TimePassedProxy, nullptr, &MockedFileSystem));
    ASSERT_THAT(GetCurrentTime(), Eq(0x1111111111111111ull));
}

TEST_F(TimerPersistanceTest, TestReadingFilesEndiannes)
{
    const uint8_t expected[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    EXPECT_CALL(fs, Open(_, _, _)).WillRepeatedly(Return(1));
    EXPECT_CALL(fs, Close(_)).WillRepeatedly(Return(0));
    EXPECT_CALL(fs, Read(_, _, _))
        .WillRepeatedly(Invoke([=](FSFileHandle /*file*/, void* buffer, unsigned int size) {
            EXPECT_THAT(size, Eq(8u));
            if (size != 8)
            {
                return -1;
            }
            else
            {
                memcpy(buffer, expected, size);
                return static_cast<int>(size);
            }
        }));
    EXPECT_TRUE(TimeInitialize(&provider, TimePassedProxy, nullptr, &MockedFileSystem));
    ASSERT_THAT(GetCurrentTime(), Eq(0x8877665544332211ull));
}

TEST_F(TimerPersistanceTest, TestReadingFilesGetClosed)
{
    EXPECT_CALL(fs, Open(_, _, _)).WillOnce(Return(1)).WillOnce(Return(2)).WillOnce(Return(3));
    EXPECT_CALL(fs, Read(_, _, _)).WillRepeatedly(Return(-1));
    EXPECT_CALL(fs, Close(1)).Times(1);
    EXPECT_CALL(fs, Close(2)).Times(1);
    EXPECT_CALL(fs, Close(3)).Times(1);
    EXPECT_TRUE(TimeInitialize(&provider, TimePassedProxy, nullptr, &MockedFileSystem));
}

TEST_F(TimerPersistanceTest, TestReadingThreeFilesTwoSame)
{
    const uint8_t expected[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    const uint8_t incorrect[] = {0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99};
    EXPECT_CALL(fs, Open(_, _, _)).WillRepeatedly(Return(1));
    EXPECT_CALL(fs, Close(_)).WillRepeatedly(Return(0));
    EXPECT_CALL(fs, Read(_, _, _))
        .WillOnce(Invoke([=](FSFileHandle /*file*/, void* buffer, unsigned int size) {
            memcpy(buffer, incorrect, size);
            return static_cast<int>(size);
        }))
        .WillRepeatedly(Invoke([=](FSFileHandle /*file*/, void* buffer, unsigned int size) {
            memcpy(buffer, expected, size);
            return static_cast<int>(size);
        }));
    EXPECT_TRUE(TimeInitialize(&provider, TimePassedProxy, nullptr, &MockedFileSystem));
    ASSERT_THAT(GetCurrentTime(), Eq(0x8877665544332211ull));
}

TEST_F(TimerPersistanceTest, TestReadingThreeDifferentFiles)
{
    const uint8_t a[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x78};
    const uint8_t b[] = {0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99};
    const uint8_t c[] = {0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa};
    EXPECT_CALL(fs, Open(_, _, _)).WillOnce(Return(1)).WillOnce(Return(2)).WillOnce(Return(3));
    EXPECT_CALL(fs, Close(_)).WillRepeatedly(Return(0));
    EXPECT_CALL(fs, Read(Eq(1), _, _))
        .WillOnce(Invoke([=](FSFileHandle /*file*/, void* buffer, unsigned int size) {
            memcpy(buffer, b, size);
            return static_cast<int>(size);
        }));

    EXPECT_CALL(fs, Read(Eq(2), _, _))
        .WillOnce(Invoke([=](FSFileHandle /*file*/, void* buffer, unsigned int size) {
            memcpy(buffer, a, size);
            return static_cast<int>(size);
        }));

    EXPECT_CALL(fs, Read(Eq(3), _, _))
        .WillOnce(Invoke([=](FSFileHandle /*file*/, void* buffer, unsigned int size) {
            memcpy(buffer, c, size);
            return static_cast<int>(size);
        }));

    EXPECT_TRUE(TimeInitialize(&provider, TimePassedProxy, nullptr, &MockedFileSystem));
    ASSERT_THAT(GetCurrentTime(), Eq(0x7877665544332211ull));
}

TEST_F(TimerPersistanceTest, TestStateSaveError)
{
    EXPECT_CALL(fs, Open(_, _, _)).WillRepeatedly(Return(-1));
    EXPECT_TRUE(TimeInitialize(&provider, TimePassedProxy, nullptr, &MockedFileSystem));
    TimeAdvanceTime(&provider, 0x44332211ull);
    ASSERT_THAT(GetCurrentTime(), Eq(0x44332211ull));
}

TEST_F(TimerPersistanceTest, TestStateWriteError)
{
    EXPECT_CALL(fs, Open(_, _, _)).WillRepeatedly(Return(-1));
    EXPECT_TRUE(TimeInitialize(&provider, TimePassedProxy, nullptr, &MockedFileSystem));
    EXPECT_CALL(fs, Open(_, _, _)).WillRepeatedly(Return(1));
    EXPECT_CALL(fs, Write(_, _, _)).WillRepeatedly(Return(-1));
    EXPECT_CALL(fs, Close(_)).Times(3).WillRepeatedly(Return(0));
    TimeAdvanceTime(&provider, 0x44332211ull);
    ASSERT_THAT(GetCurrentTime(), Eq(0x44332211ull));
}

TEST_F(TimerPersistanceTest, TestStateWrite)
{
    EXPECT_CALL(fs, Open(_, _, _)).WillRepeatedly(Return(-1));
    EXPECT_TRUE(TimeInitialize(&provider, TimePassedProxy, nullptr, &MockedFileSystem));
    EXPECT_CALL(fs, Open(_, _, _)).WillRepeatedly(Return(1));
    EXPECT_CALL(fs, Close(_)).WillRepeatedly(Return(0));
    EXPECT_CALL(fs, Write(_, _, _))
        .WillRepeatedly(Invoke([](FSFileHandle /*file*/, const void* buffer, unsigned int size) {
            uint8_t expected[] = {0x11, 0x22, 0x33, 0x44, 0x00, 0x00, 0x00, 0x00};
            EXPECT_THAT(size, Eq(8u));
            EXPECT_THAT(std::vector<uint8_t>(static_cast<const uint8_t*>(buffer), static_cast<const uint8_t*>(buffer) + size),
                ::testing::ElementsAreArray(expected));
            return static_cast<int>(size);
        }));
    TimeAdvanceTime(&provider, 0x44332211ull);
    ASSERT_THAT(GetCurrentTime(), Eq(0x44332211ull));
}

TEST_F(TimerPersistanceTest, TestStateWriteClosesFiles)
{
    EXPECT_CALL(fs, Open(_, _, _)).WillRepeatedly(Return(-1));
    EXPECT_TRUE(TimeInitialize(&provider, TimePassedProxy, nullptr, &MockedFileSystem));
    EXPECT_CALL(fs, Open(_, _, _)).WillOnce(Return(1)).WillOnce(Return(2)).WillOnce(Return(3));
    EXPECT_CALL(fs, Close(1)).Times(1);
    EXPECT_CALL(fs, Close(2)).Times(1);
    EXPECT_CALL(fs, Close(3)).Times(1);
    EXPECT_CALL(fs, Write(_, _, _)).WillRepeatedly(Return(-1));
    TimeAdvanceTime(&provider, 0x44332211ull);
    ASSERT_THAT(GetCurrentTime(), Eq(0x44332211ull));
}

TEST_F(TimerPersistanceTest, TestStateWriteIsNotDoneOnEveryTimeUpdate)
{
    EXPECT_CALL(fs, Open(_, _, _)).WillRepeatedly(Return(-1));
    EXPECT_TRUE(TimeInitialize(&provider, TimePassedProxy, nullptr, &MockedFileSystem));
    EXPECT_CALL(fs, Open(_, _, _)).Times(3).WillRepeatedly(Return(1));
    EXPECT_CALL(fs, Close(_)).Times(3);
    EXPECT_CALL(fs, Write(_, _, _)).WillRepeatedly(Return(-1));

    TimeAdvanceTime(&provider, 400000);
    TimeAdvanceTime(&provider, 100000);
    TimeAdvanceTime(&provider, 300000);
    TimeAdvanceTime(&provider, 200000);
    ASSERT_THAT(GetCurrentTime(), Eq(1000000ull));
}
