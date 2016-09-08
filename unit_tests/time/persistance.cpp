#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "OsMock.hpp"
#include "mock/FsMock.hpp"
#include "os/os.hpp"

#include "time/timer.h"
#include "TimeSpan.hpp"

using testing::_;
using testing::Eq;
using testing::Ne;
using testing::Return;
using testing::Invoke;
using testing::HasSubstr;

class TimerPersistanceTest : public testing::Test
{
  protected:
    TimerPersistanceTest();
    TimeSpan GetCurrentTime();

    TimeProvider provider;
    testing::NiceMock<FsMock> fs;
    testing::NiceMock<OSMock> os;
    OSReset osGuard;
};

TimeSpan TimerPersistanceTest::GetCurrentTime()
{
    TimeSpan span{0};
    EXPECT_TRUE(TimeGetCurrentTime(&provider, &span));
    return span;
}

static void TimePassedProxy(void* /*context*/, TimePoint /*currentTime*/)
{
}

TimerPersistanceTest::TimerPersistanceTest()
{
    this->osGuard = InstallProxy(&os);
    EXPECT_CALL(os, CreateBinarySemaphore()).WillOnce(Return(reinterpret_cast<void*>(1))).WillOnce(Return(reinterpret_cast<void*>(2)));
    ON_CALL(os, TakeSemaphore(_, _)).WillByDefault(Return(OSResultSuccess));
    ON_CALL(os, GiveSemaphore(_)).WillByDefault(Return(OSResultSuccess));
}

TEST_F(TimerPersistanceTest, TestReadingStateNoState)
{
    EXPECT_CALL(fs, Open(_, _, _)).WillRepeatedly(Return(MakeOpenedFile(OSResultNotFound)));
    EXPECT_TRUE(TimeInitialize(&provider, TimePassedProxy, nullptr, &fs));
    ASSERT_THAT(GetCurrentTime(), Eq(TimeSpanFromMilliseconds(0u)));
}

TEST_F(TimerPersistanceTest, TestReadingStateEmptyFiles)
{
    EXPECT_CALL(fs, Open(_, _, _)).WillRepeatedly(Return(MakeOpenedFile(1)));
    EXPECT_CALL(fs, Read(_, _, _)).WillRepeatedly(Return(MakeFSIOResult(OSResultInvalidOperation)));
    EXPECT_CALL(fs, Close(_)).WillRepeatedly(Return(OSResultSuccess));
    EXPECT_TRUE(TimeInitialize(&provider, TimePassedProxy, nullptr, &fs));
    ASSERT_THAT(GetCurrentTime(), Eq(TimeSpanFromMilliseconds(0u)));
}

TEST_F(TimerPersistanceTest, TestReadingFiles)
{
    EXPECT_CALL(fs, Open(_, _, _)).WillRepeatedly(Return(MakeOpenedFile(10)));
    ON_CALL(fs, Close(_)).WillByDefault(Return(OSResultSuccess));
    EXPECT_CALL(fs, Read(_, _, _))
        .WillRepeatedly(Invoke([](FSFileHandle /*file*/, void* buffer, FSFileSize size) {
            EXPECT_THAT(buffer, Ne(nullptr));
            if (buffer == nullptr)
            {
                return MakeFSIOResult(OSResultInvalidOperation);
            }
            else
            {
                memset(buffer, 0x11, size);
                return MakeFSIOResult(size);
            }
        }));
    EXPECT_TRUE(TimeInitialize(&provider, TimePassedProxy, nullptr, &fs));
    ASSERT_THAT(GetCurrentTime(), Eq(TimeSpanFromMilliseconds(0x1111111111111111ull)));
}

TEST_F(TimerPersistanceTest, TestReadingSingleNonEmptyFile)
{
    EXPECT_CALL(fs, Open(_, _, _)).WillOnce(Return(MakeOpenedFile(1))).WillRepeatedly(Return(MakeOpenedFile(OSResultNotFound)));
    ON_CALL(fs, Close(_)).WillByDefault(Return(OSResultSuccess));
    EXPECT_CALL(fs, Read(_, _, _))
        .WillOnce(Invoke([](FSFileHandle /*file*/, void* buffer, FSFileSize size) {
            memset(buffer, 0x11, size);
            return MakeFSIOResult(size);
        }))
        .WillRepeatedly(Return(MakeFSIOResult(OSResultInvalidOperation)));
    EXPECT_TRUE(TimeInitialize(&provider, TimePassedProxy, nullptr, &fs));
    ASSERT_THAT(GetCurrentTime(), Eq(TimeSpanFromMilliseconds(0u)));
}

TEST_F(TimerPersistanceTest, TestReadingTwoNonEmptyFiles)
{
    EXPECT_CALL(fs, Open(_, _, _)).WillRepeatedly(Return(MakeOpenedFile(1)));
    ON_CALL(fs, Close(_)).WillByDefault(Return(OSResultSuccess));
    EXPECT_CALL(fs, Read(_, _, _))
        .WillOnce(Return(MakeFSIOResult(0)))
        .WillRepeatedly(Invoke([](FSFileHandle /*file*/, void* buffer, FSFileSize size) {
            memset(buffer, 0x11, size);
            return MakeFSIOResult(size);
        }));
    EXPECT_TRUE(TimeInitialize(&provider, TimePassedProxy, nullptr, &fs));
    ASSERT_THAT(GetCurrentTime(), Eq(TimeSpanFromMilliseconds(0x1111111111111111ull)));
}

TEST_F(TimerPersistanceTest, TestReadingTwoExistingEmptyFiles)
{
    EXPECT_CALL(fs, Open(_, _, _)).WillOnce(Return(MakeOpenedFile(OSResultNotFound))).WillRepeatedly(Return(MakeOpenedFile(1)));
    ON_CALL(fs, Close(_)).WillByDefault(Return(OSResultSuccess));
    EXPECT_CALL(fs, Read(_, _, _))
        .WillRepeatedly(Invoke([](FSFileHandle /*file*/, void* buffer, FSFileSize size) {
            memset(buffer, 0x11, size);
            return MakeFSIOResult(size);
        }));
    EXPECT_TRUE(TimeInitialize(&provider, TimePassedProxy, nullptr, &fs));
    ASSERT_THAT(GetCurrentTime(), Eq(TimeSpanFromMilliseconds(0x1111111111111111ull)));
}

TEST_F(TimerPersistanceTest, TestReadingFilesEndiannes)
{
    const uint8_t expected[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    EXPECT_CALL(fs, Open(_, _, _)).WillRepeatedly(Return(MakeOpenedFile(1)));
    ON_CALL(fs, Close(_)).WillByDefault(Return(OSResultSuccess));
    EXPECT_CALL(fs, Read(_, _, _))
        .WillRepeatedly(Invoke([=](FSFileHandle /*file*/, void* buffer, FSFileSize size) {
            EXPECT_THAT(size, Eq(8));
            if (size != 8)
            {
                return MakeFSIOResult(OSResultInvalidOperation);
            }
            else
            {
                memcpy(buffer, expected, size);
                return MakeFSIOResult(size);
            }
        }));
    EXPECT_TRUE(TimeInitialize(&provider, TimePassedProxy, nullptr, &fs));
    ASSERT_THAT(GetCurrentTime(), Eq(TimeSpanFromMilliseconds(0x8877665544332211ull)));
}

TEST_F(TimerPersistanceTest, TestReadingFilesGetClosed)
{
    EXPECT_CALL(fs, Open(_, _, _))
        .WillOnce(Return(MakeOpenedFile(1)))
        .WillOnce(Return(MakeOpenedFile(2)))
        .WillOnce(Return(MakeOpenedFile(3)));
    EXPECT_CALL(fs, Read(_, _, _)).WillRepeatedly(Return(MakeFSIOResult(OSResultNotEnoughMemory)));
    EXPECT_CALL(fs, Close(1)).Times(1);
    EXPECT_CALL(fs, Close(2)).Times(1);
    EXPECT_CALL(fs, Close(3)).Times(1);
    EXPECT_TRUE(TimeInitialize(&provider, TimePassedProxy, nullptr, &fs));
}

TEST_F(TimerPersistanceTest, TestReadingThreeFilesTwoSame)
{
    const uint8_t expected[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    const uint8_t incorrect[] = {0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99};
    EXPECT_CALL(fs, Open(_, _, _)).WillRepeatedly(Return(MakeOpenedFile(1)));
    ON_CALL(fs, Close(_)).WillByDefault(Return(OSResultSuccess));
    EXPECT_CALL(fs, Read(_, _, _))
        .WillOnce(Invoke([=](FSFileHandle /*file*/, void* buffer, FSFileSize size) {
            memcpy(buffer, incorrect, size);
            return MakeFSIOResult(size);
        }))
        .WillRepeatedly(Invoke([=](FSFileHandle /*file*/, void* buffer, FSFileSize size) {
            memcpy(buffer, expected, size);
            return MakeFSIOResult(size);
        }));
    EXPECT_TRUE(TimeInitialize(&provider, TimePassedProxy, nullptr, &fs));
    ASSERT_THAT(GetCurrentTime(), Eq(TimeSpanFromMilliseconds(0x8877665544332211ull)));
}

TEST_F(TimerPersistanceTest, TestReadingThreeDifferentFiles)
{
    const uint8_t a[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x78};
    const uint8_t b[] = {0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99};
    const uint8_t c[] = {0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa};
    EXPECT_CALL(fs, Open(_, _, _))
        .WillOnce(Return(MakeOpenedFile(1)))
        .WillOnce(Return(MakeOpenedFile(2)))
        .WillOnce(Return(MakeOpenedFile(3)));
    ON_CALL(fs, Close(_)).WillByDefault(Return(OSResultSuccess));
    EXPECT_CALL(fs, Read(Eq(1), _, _))
        .WillOnce(Invoke([=](FSFileHandle /*file*/, void* buffer, FSFileSize size) {
            memcpy(buffer, b, size);
            return MakeFSIOResult(size);
        }));

    EXPECT_CALL(fs, Read(Eq(2), _, _))
        .WillOnce(Invoke([=](FSFileHandle /*file*/, void* buffer, FSFileSize size) {
            memcpy(buffer, a, size);
            return MakeFSIOResult(size);
        }));

    EXPECT_CALL(fs, Read(Eq(3), _, _))
        .WillOnce(Invoke([=](FSFileHandle /*file*/, void* buffer, FSFileSize size) {
            memcpy(buffer, c, size);
            return MakeFSIOResult(size);
        }));

    EXPECT_TRUE(TimeInitialize(&provider, TimePassedProxy, nullptr, &fs));
    ASSERT_THAT(GetCurrentTime(), Eq(TimeSpanFromMilliseconds(0x7877665544332211ull)));
}

TEST_F(TimerPersistanceTest, TestStateSaveError)
{
    EXPECT_CALL(fs, Open(_, _, _)).WillRepeatedly(Return(MakeOpenedFile(OSResultNotFound)));
    EXPECT_TRUE(TimeInitialize(&provider, TimePassedProxy, nullptr, &fs));
    TimeAdvanceTime(&provider, TimeSpanFromMilliseconds(0x44332211ull));
    ASSERT_THAT(GetCurrentTime(), Eq(TimeSpanFromMilliseconds(0x44332211ull)));
}

TEST_F(TimerPersistanceTest, TestStateWriteError)
{
    EXPECT_CALL(fs, Open(_, _, _)).WillRepeatedly(Return(MakeOpenedFile(OSResultNotFound)));
    EXPECT_TRUE(TimeInitialize(&provider, TimePassedProxy, nullptr, &fs));
    EXPECT_CALL(fs, Open(_, _, _)).WillRepeatedly(Return(MakeOpenedFile(1)));
    EXPECT_CALL(fs, Write(_, _, _)).WillRepeatedly(Return(MakeFSIOResult(OSResultInvalidOperation)));
    ON_CALL(fs, Close(_)).WillByDefault(Return(OSResultSuccess));
    TimeAdvanceTime(&provider, TimeSpanFromMilliseconds(0x44332211ull));
    ASSERT_THAT(GetCurrentTime(), Eq(TimeSpanFromMilliseconds(0x44332211ull)));
}

TEST_F(TimerPersistanceTest, TestStateWrite)
{
    EXPECT_CALL(fs, Open(_, _, _)).WillRepeatedly(Return(MakeOpenedFile(OSResultNotFound)));
    EXPECT_TRUE(TimeInitialize(&provider, TimePassedProxy, nullptr, &fs));
    EXPECT_CALL(fs, Open(_, _, _)).WillRepeatedly(Return(MakeOpenedFile(1)));
    ON_CALL(fs, Close(_)).WillByDefault(Return(OSResultSuccess));
    EXPECT_CALL(fs, Write(_, _, _))
        .WillRepeatedly(Invoke([](FSFileHandle /*file*/, const void* buffer, FSFileSize size) {
            uint8_t expected[] = {0x11, 0x22, 0x33, 0x44, 0x00, 0x00, 0x00, 0x00};
            EXPECT_THAT(size, Eq(8));
            EXPECT_THAT(std::vector<uint8_t>(static_cast<const uint8_t*>(buffer), static_cast<const uint8_t*>(buffer) + size),
                ::testing::ElementsAreArray(expected));
            return MakeFSIOResult(size);
        }));
    TimeAdvanceTime(&provider, TimeSpanFromMilliseconds(0x44332211ull));
    ASSERT_THAT(GetCurrentTime(), Eq(TimeSpanFromMilliseconds(0x44332211ull)));
}

TEST_F(TimerPersistanceTest, TestStateWriteClosesFiles)
{
    EXPECT_CALL(fs, Open(_, _, _)).WillRepeatedly(Return(MakeOpenedFile(OSResultNotFound)));
    EXPECT_TRUE(TimeInitialize(&provider, TimePassedProxy, nullptr, &fs));
    EXPECT_CALL(fs, Open(HasSubstr(".0"), _, _)).Times(3).WillRepeatedly(Return(MakeOpenedFile(1)));
    EXPECT_CALL(fs, Open(HasSubstr(".1"), _, _)).Times(3).WillRepeatedly(Return(MakeOpenedFile(2)));
    EXPECT_CALL(fs, Open(HasSubstr(".2"), _, _)).Times(3).WillRepeatedly(Return(MakeOpenedFile(3)));
    EXPECT_CALL(fs, Close(1)).Times(3);
    EXPECT_CALL(fs, Close(2)).Times(3);
    EXPECT_CALL(fs, Close(3)).Times(3);
    EXPECT_CALL(fs, Write(_, _, _)).WillRepeatedly(Return(MakeFSIOResult(OSResultInvalidOperation)));
    TimeAdvanceTime(&provider, TimeSpanFromMilliseconds(0x44332211ull));
    ASSERT_THAT(GetCurrentTime(), Eq(TimeSpanFromMilliseconds(0x44332211ull)));
}

TEST_F(TimerPersistanceTest, TestStateWriteIsNotDoneOnEveryTimeUpdate)
{
    EXPECT_CALL(fs, Open(_, _, _)).WillRepeatedly(Return(MakeOpenedFile(OSResultNotFound)));
    EXPECT_TRUE(TimeInitialize(&provider, TimePassedProxy, nullptr, &fs));
    EXPECT_CALL(fs, Open(_, _, _)).WillRepeatedly(Return(MakeOpenedFile(1)));
    EXPECT_CALL(fs, Close(_)).Times(9);
    EXPECT_CALL(fs, Write(_, _, _)).WillRepeatedly(Return(MakeFSIOResult(OSResultInvalidOperation)));

    TimeAdvanceTime(&provider, TimeSpanFromMilliseconds(400000));
    TimeAdvanceTime(&provider, TimeSpanFromMilliseconds(100000));
    TimeAdvanceTime(&provider, TimeSpanFromMilliseconds(300000));
    TimeAdvanceTime(&provider, TimeSpanFromMilliseconds(200000));
    ASSERT_THAT(GetCurrentTime(), Eq(TimeSpanFromMilliseconds(1000000ull)));
}
