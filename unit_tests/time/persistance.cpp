#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "OsMock.hpp"
#include "mock/FsMock.hpp"
#include "os/os.hpp"
#include "utils.hpp"

#include "TimeSpan.hpp"
#include "time/timer.h"

using testing::_;
using testing::Eq;
using testing::Ne;
using testing::Return;
using testing::Invoke;
using testing::HasSubstr;
using testing::ElementsAreArray;
using services::time::TimeProvider;

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
    Option<TimeSpan> span = provider.GetCurrentTime();
    EXPECT_TRUE(span.HasValue);
    return span.Value;
}

static void TimePassedProxy(void* /*context*/, TimePoint /*currentTime*/)
{
}

TimerPersistanceTest::TimerPersistanceTest() : provider(fs)
{
    this->osGuard = InstallProxy(&os);
    EXPECT_CALL(os, CreateBinarySemaphore(_)).WillOnce(Return(reinterpret_cast<void*>(1))).WillOnce(Return(reinterpret_cast<void*>(2)));
    ON_CALL(os, TakeSemaphore(_, _)).WillByDefault(Return(OSResult::Success));
    ON_CALL(os, GiveSemaphore(_)).WillByDefault(Return(OSResult::Success));

    EXPECT_CALL(os, CreatePulseAll()).WillOnce(Return(reinterpret_cast<void*>(3)));
}

TEST_F(TimerPersistanceTest, TestReadingStateNoState)
{
    EXPECT_CALL(fs, Open(_, _, _)).WillRepeatedly(Return(MakeOpenedFile(OSResult::NotFound)));
    EXPECT_TRUE(provider.Initialize(TimePassedProxy, nullptr));
    ASSERT_THAT(GetCurrentTime(), Eq(TimeSpanFromMilliseconds(0u)));
}

TEST_F(TimerPersistanceTest, TestReadingStateEmptyFiles)
{
    EXPECT_CALL(fs, Open(_, _, _)).WillRepeatedly(Return(MakeOpenedFile(1)));
    EXPECT_CALL(fs, Read(_, _)).WillRepeatedly(Return(MakeFSIOResult(OSResult::InvalidOperation)));
    EXPECT_CALL(fs, Close(_)).WillRepeatedly(Return(OSResult::Success));
    EXPECT_TRUE(provider.Initialize(TimePassedProxy, nullptr));
    ASSERT_THAT(GetCurrentTime(), Eq(TimeSpanFromMilliseconds(0u)));
}

TEST_F(TimerPersistanceTest, TestReadingFiles)
{
    EXPECT_CALL(fs, Open(_, _, _)).WillRepeatedly(Return(MakeOpenedFile(10)));
    ON_CALL(fs, Close(_)).WillByDefault(Return(OSResult::Success));
    EXPECT_CALL(fs, Read(_, _)).WillRepeatedly(Invoke([](FSFileHandle /*file*/, gsl::span<std::uint8_t> buffer) {
        std::fill(buffer.begin(), buffer.end(), 0x11);
        return MakeFSIOResult(buffer.size());
    }));
    EXPECT_TRUE(provider.Initialize(TimePassedProxy, nullptr));
    ASSERT_THAT(GetCurrentTime(), Eq(TimeSpanFromMilliseconds(0x1111111111111111ull)));
}

TEST_F(TimerPersistanceTest, TestReadingSingleNonEmptyFile)
{
    EXPECT_CALL(fs, Open(_, _, _)).WillOnce(Return(MakeOpenedFile(1))).WillRepeatedly(Return(MakeOpenedFile(OSResult::NotFound)));
    ON_CALL(fs, Close(_)).WillByDefault(Return(OSResult::Success));
    EXPECT_CALL(fs, Read(_, _))
        .WillOnce(Invoke([](FSFileHandle /*file*/, gsl::span<std::uint8_t> buffer) {
            std::fill(buffer.begin(), buffer.end(), 0x11);
            return MakeFSIOResult(buffer.size());
        }))
        .WillRepeatedly(Return(MakeFSIOResult(OSResult::InvalidOperation)));
    EXPECT_TRUE(provider.Initialize(TimePassedProxy, nullptr));
    ASSERT_THAT(GetCurrentTime(), Eq(TimeSpanFromMilliseconds(0u)));
}

TEST_F(TimerPersistanceTest, TestReadingTwoNonEmptyFiles)
{
    EXPECT_CALL(fs, Open(_, _, _)).WillRepeatedly(Return(MakeOpenedFile(1)));
    ON_CALL(fs, Close(_)).WillByDefault(Return(OSResult::Success));
    EXPECT_CALL(fs, Read(_, _))
        .WillOnce(Return(MakeFSIOResult(0)))
        .WillRepeatedly(Invoke([](FSFileHandle /*file*/, gsl::span<std::uint8_t> buffer) {
            std::fill(buffer.begin(), buffer.end(), 0x11);
            return MakeFSIOResult(buffer.size());
        }));
    EXPECT_TRUE(provider.Initialize(TimePassedProxy, nullptr));
    ASSERT_THAT(GetCurrentTime(), Eq(TimeSpanFromMilliseconds(0x1111111111111111ull)));
}

TEST_F(TimerPersistanceTest, TestReadingTwoExistingEmptyFiles)
{
    EXPECT_CALL(fs, Open(_, _, _)).WillOnce(Return(MakeOpenedFile(OSResult::NotFound))).WillRepeatedly(Return(MakeOpenedFile(1)));
    ON_CALL(fs, Close(_)).WillByDefault(Return(OSResult::Success));
    EXPECT_CALL(fs, Read(_, _)).WillRepeatedly(Invoke([](FSFileHandle /*file*/, gsl::span<std::uint8_t> buffer) {
        std::fill(buffer.begin(), buffer.end(), 0x11);
        return MakeFSIOResult(buffer.size());
    }));
    EXPECT_TRUE(provider.Initialize(TimePassedProxy, nullptr));
    ASSERT_THAT(GetCurrentTime(), Eq(TimeSpanFromMilliseconds(0x1111111111111111ull)));
}

TEST_F(TimerPersistanceTest, TestReadingFilesEndiannes)
{
    std::array<const uint8_t, 8> expected{0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};

    EXPECT_CALL(fs, Open(_, _, _)).WillRepeatedly(Return(MakeOpenedFile(1)));
    ON_CALL(fs, Close(_)).WillByDefault(Return(OSResult::Success));
    EXPECT_CALL(fs, Read(_, _)).WillRepeatedly(Invoke([=](FSFileHandle /*file*/, gsl::span<std::uint8_t> buffer) {
        EXPECT_THAT(buffer, SpanOfSize(8));
        if (buffer.size() != 8)
        {
            return MakeFSIOResult(OSResult::InvalidOperation);
        }
        else
        {
            std::copy(expected.begin(), expected.end(), buffer.begin());
            return MakeFSIOResult(buffer.size());
        }
    }));
    EXPECT_TRUE(provider.Initialize(TimePassedProxy, nullptr));
    ASSERT_THAT(GetCurrentTime(), Eq(TimeSpanFromMilliseconds(0x8877665544332211ull)));
}

TEST_F(TimerPersistanceTest, TestReadingFilesGetClosed)
{
    EXPECT_CALL(fs, Open(_, _, _))
        .WillOnce(Return(MakeOpenedFile(1)))
        .WillOnce(Return(MakeOpenedFile(2)))
        .WillOnce(Return(MakeOpenedFile(3)));
    EXPECT_CALL(fs, Read(_, _)).WillRepeatedly(Return(MakeFSIOResult(OSResult::NotEnoughMemory)));
    EXPECT_CALL(fs, Close(1)).Times(1);
    EXPECT_CALL(fs, Close(2)).Times(1);
    EXPECT_CALL(fs, Close(3)).Times(1);
    EXPECT_TRUE(provider.Initialize(TimePassedProxy, nullptr));
}

TEST_F(TimerPersistanceTest, TestReadingThreeFilesTwoSame)
{
    std::array<const uint8_t, 8> expected{0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    std::array<const uint8_t, 8> incorrect{0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99};
    EXPECT_CALL(fs, Open(_, _, _)).WillRepeatedly(Return(MakeOpenedFile(1)));
    ON_CALL(fs, Close(_)).WillByDefault(Return(OSResult::Success));
    EXPECT_CALL(fs, Read(_, _))
        .WillOnce(Invoke([=](FSFileHandle /*file*/, gsl::span<std::uint8_t> buffer) {
            std::copy(incorrect.begin(), incorrect.end(), buffer.begin());
            return MakeFSIOResult(buffer.size());
        }))
        .WillRepeatedly(Invoke([=](FSFileHandle /*file*/, gsl::span<std::uint8_t> buffer) {
            std::copy(expected.begin(), expected.end(), buffer.begin());
            return MakeFSIOResult(buffer.size());
        }));
    EXPECT_TRUE(provider.Initialize(TimePassedProxy, nullptr));
    ASSERT_THAT(GetCurrentTime(), Eq(TimeSpanFromMilliseconds(0x8877665544332211ull)));
}

TEST_F(TimerPersistanceTest, TestReadingThreeDifferentFiles)
{
    std::array<const uint8_t, 8> a{0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x78};
    std::array<const uint8_t, 8> b{0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99};
    std::array<const uint8_t, 8> c{0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa};
    EXPECT_CALL(fs, Open(_, _, _))
        .WillOnce(Return(MakeOpenedFile(1)))
        .WillOnce(Return(MakeOpenedFile(2)))
        .WillOnce(Return(MakeOpenedFile(3)));
    ON_CALL(fs, Close(_)).WillByDefault(Return(OSResult::Success));
    EXPECT_CALL(fs, Read(Eq(1), _)).WillOnce(Invoke([=](FSFileHandle /*file*/, gsl::span<std::uint8_t> buffer) {
        std::copy(b.begin(), b.end(), buffer.begin());
        return MakeFSIOResult(buffer.size());
    }));

    EXPECT_CALL(fs, Read(Eq(2), _)).WillOnce(Invoke([=](FSFileHandle /*file*/, gsl::span<std::uint8_t> buffer) {
        std::copy(a.begin(), a.end(), buffer.begin());
        return MakeFSIOResult(buffer.size());
    }));

    EXPECT_CALL(fs, Read(Eq(3), _)).WillOnce(Invoke([=](FSFileHandle /*file*/, gsl::span<std::uint8_t> buffer) {
        std::copy(c.begin(), c.end(), buffer.begin());
        return MakeFSIOResult(buffer.size());
    }));

    EXPECT_TRUE(provider.Initialize(TimePassedProxy, nullptr));
    ASSERT_THAT(GetCurrentTime(), Eq(TimeSpanFromMilliseconds(0x7877665544332211ull)));
}

TEST_F(TimerPersistanceTest, TestStateSaveError)
{
    EXPECT_CALL(fs, Open(_, _, _)).WillRepeatedly(Return(MakeOpenedFile(OSResult::NotFound)));
    EXPECT_TRUE(provider.Initialize(TimePassedProxy, nullptr));
    provider.AdvanceTime(TimeSpanFromMilliseconds(0x44332211ull));
    ASSERT_THAT(GetCurrentTime(), Eq(TimeSpanFromMilliseconds(0x44332211ull)));
}

TEST_F(TimerPersistanceTest, TestStateWriteError)
{
    EXPECT_CALL(fs, Open(_, _, _)).WillRepeatedly(Return(MakeOpenedFile(OSResult::NotFound)));
    EXPECT_TRUE(provider.Initialize(TimePassedProxy, nullptr));
    EXPECT_CALL(fs, Open(_, _, _)).WillRepeatedly(Return(MakeOpenedFile(1)));
    EXPECT_CALL(fs, Write(_, _)).WillRepeatedly(Return(MakeFSIOResult(OSResult::InvalidOperation)));
    ON_CALL(fs, Close(_)).WillByDefault(Return(OSResult::Success));
    provider.AdvanceTime(TimeSpanFromMilliseconds(0x44332211ull));
    ASSERT_THAT(GetCurrentTime(), Eq(TimeSpanFromMilliseconds(0x44332211ull)));
}

TEST_F(TimerPersistanceTest, TestStateWrite)
{
    EXPECT_CALL(fs, Open(_, _, _)).WillRepeatedly(Return(MakeOpenedFile(OSResult::NotFound)));
    EXPECT_TRUE(provider.Initialize(TimePassedProxy, nullptr));
    EXPECT_CALL(fs, Open(_, _, _)).WillRepeatedly(Return(MakeOpenedFile(1)));
    ON_CALL(fs, Close(_)).WillByDefault(Return(OSResult::Success));
    EXPECT_CALL(fs, Write(_, _)).WillRepeatedly(Invoke([](FSFileHandle /*file*/, gsl::span<const std::uint8_t> buffer) {
        uint8_t expected[] = {0x11, 0x22, 0x33, 0x44, 0x00, 0x00, 0x00, 0x00};
        EXPECT_THAT(buffer, ElementsAreArray(expected));

        return MakeFSIOResult(buffer.size());
    }));
    provider.AdvanceTime(TimeSpanFromMilliseconds(0x44332211ull));
    ASSERT_THAT(GetCurrentTime(), Eq(TimeSpanFromMilliseconds(0x44332211ull)));
}

TEST_F(TimerPersistanceTest, TestStateWriteClosesFiles)
{
    EXPECT_CALL(fs, Open(_, _, _)).WillRepeatedly(Return(MakeOpenedFile(OSResult::NotFound)));
    EXPECT_TRUE(provider.Initialize(TimePassedProxy, nullptr));
    EXPECT_CALL(fs, Open(HasSubstr(".0"), _, _)).Times(3).WillRepeatedly(Return(MakeOpenedFile(1)));
    EXPECT_CALL(fs, Open(HasSubstr(".1"), _, _)).Times(3).WillRepeatedly(Return(MakeOpenedFile(2)));
    EXPECT_CALL(fs, Open(HasSubstr(".2"), _, _)).Times(3).WillRepeatedly(Return(MakeOpenedFile(3)));
    EXPECT_CALL(fs, Close(1)).Times(3);
    EXPECT_CALL(fs, Close(2)).Times(3);
    EXPECT_CALL(fs, Close(3)).Times(3);
    EXPECT_CALL(fs, Write(_, _)).WillRepeatedly(Return(MakeFSIOResult(OSResult::InvalidOperation)));
    provider.AdvanceTime(TimeSpanFromMilliseconds(0x44332211ull));
    ASSERT_THAT(GetCurrentTime(), Eq(TimeSpanFromMilliseconds(0x44332211ull)));
}

TEST_F(TimerPersistanceTest, TestStateWriteIsNotDoneOnEveryTimeUpdate)
{
    EXPECT_CALL(fs, Open(_, _, _)).WillRepeatedly(Return(MakeOpenedFile(OSResult::NotFound)));
    EXPECT_TRUE(provider.Initialize(TimePassedProxy, nullptr));
    EXPECT_CALL(fs, Open(_, _, _)).WillRepeatedly(Return(MakeOpenedFile(1)));
    EXPECT_CALL(fs, Close(_)).Times(9);
    EXPECT_CALL(fs, Write(_, _)).WillRepeatedly(Return(MakeFSIOResult(OSResult::InvalidOperation)));

    provider.AdvanceTime(TimeSpanFromMilliseconds(400000));
    provider.AdvanceTime(TimeSpanFromMilliseconds(100000));
    provider.AdvanceTime(TimeSpanFromMilliseconds(300000));
    provider.AdvanceTime(TimeSpanFromMilliseconds(200000));
    ASSERT_THAT(GetCurrentTime(), Eq(TimeSpanFromMilliseconds(1000000ull)));
}
