#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "comm/comm.h"

using namespace devices::comm;

using testing::Eq;

TEST(CommFrameTest, TestEmptyFrame)
{
    CommFrame frame;
    ASSERT_THAT(frame.Size(), Eq(0));
    ASSERT_THAT(frame.FullSize(), Eq(0));
    ASSERT_THAT(frame.Rssi(), Eq(0));
    ASSERT_THAT(frame.Doppler(), Eq(0));
    ASSERT_THAT(frame.Payload().empty(), Eq(true));
}

TEST(CommFrameTest, TestSimpleFrame)
{
    std::uint8_t buffer[10];
    CommFrame frame(1, 2, 10, buffer);
    ASSERT_THAT(frame.Size(), Eq(10));
    ASSERT_THAT(frame.FullSize(), Eq(10));
    ASSERT_THAT(frame.Rssi(), Eq(2));
    ASSERT_THAT(frame.Doppler(), Eq(1));
    ASSERT_THAT(frame.IsComplete(), Eq(true));
    ASSERT_THAT(frame.Payload().empty(), Eq(false));
    ASSERT_THAT(frame.Payload().data(), Eq(buffer));
}

TEST(CommFrameTest, TestSimpleEmptyFrame)
{
    CommFrame frame(1, 2, 0, gsl::span<std::uint8_t>());
    ASSERT_THAT(frame.Size(), Eq(0));
    ASSERT_THAT(frame.FullSize(), Eq(0));
    ASSERT_THAT(frame.Payload().empty(), Eq(true));
}

TEST(CommFrameTest, TestSimpleFrameRssiVerification)
{
    std::uint8_t buffer[10];
    CommFrame frame(1, 0xfff, 10, buffer);
    ASSERT_THAT(frame.IsRssiValid(), Eq(true));
    ASSERT_THAT(frame.Verify(), Eq(true));
}

TEST(CommFrameTest, TestSimpleFrameRssiVerificationFailure)
{
    std::uint8_t buffer[10];
    CommFrame frame(1, 0xf000, 10, buffer);
    ASSERT_THAT(frame.IsRssiValid(), Eq(false));
    ASSERT_THAT(frame.Verify(), Eq(false));
}

TEST(CommFrameTest, TestSimpleFrameDoppleVerification)
{
    std::uint8_t buffer[10];
    CommFrame frame(0xfff, 1, 10, buffer);
    ASSERT_THAT(frame.IsDopplerValid(), Eq(true));
    ASSERT_THAT(frame.Verify(), Eq(true));
}

TEST(CommFrameTest, TestSimpleFrameDopplerVerificationFailure)
{
    std::uint8_t buffer[10];
    CommFrame frame(0xf000, 1, 10, buffer);
    ASSERT_THAT(frame.IsDopplerValid(), Eq(false));
    ASSERT_THAT(frame.Verify(), Eq(false));
}

TEST(CommFrameTest, TestPartialFrame)
{
    std::uint8_t buffer[10];
    CommFrame frame(0xf000, 1, 16, buffer);
    ASSERT_THAT(frame.Size(), Eq(10));
    ASSERT_THAT(frame.FullSize(), Eq(16));
    ASSERT_THAT(frame.Payload().empty(), Eq(false));
    ASSERT_THAT(frame.Payload().data(), Eq(buffer));
    ASSERT_THAT(frame.Payload().size(), Eq(10));
    ASSERT_THAT(frame.IsComplete(), Eq(false));
    ASSERT_THAT(frame.Verify(), Eq(false));
}
