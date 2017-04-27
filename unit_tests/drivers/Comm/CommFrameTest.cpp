#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "comm/Frame.hpp"

using namespace devices::comm;

using testing::Eq;

namespace
{
    TEST(FrameTest, TestEmptyFrame)
    {
        Frame frame;
        ASSERT_THAT(frame.Size(), Eq(0));
        ASSERT_THAT(frame.FullSize(), Eq(0));
        ASSERT_THAT(frame.Rssi(), Eq(0));
        ASSERT_THAT(frame.Doppler(), Eq(0));
        ASSERT_THAT(frame.Payload().empty(), Eq(true));
    }

    TEST(FrameTest, TestSimpleFrame)
    {
        std::uint8_t buffer[10];
        Frame frame(1, 2, 10, buffer);
        ASSERT_THAT(frame.Size(), Eq(10));
        ASSERT_THAT(frame.FullSize(), Eq(10));
        ASSERT_THAT(frame.Rssi(), Eq(2));
        ASSERT_THAT(frame.Doppler(), Eq(1));
        ASSERT_THAT(frame.IsComplete(), Eq(true));
        ASSERT_THAT(frame.Payload().empty(), Eq(false));
        ASSERT_THAT(frame.Payload().data(), Eq(buffer));
    }

    TEST(FrameTest, TestSimpleEmptyFrame)
    {
        Frame frame(1, 2, 0, gsl::span<std::uint8_t>());
        ASSERT_THAT(frame.Size(), Eq(0));
        ASSERT_THAT(frame.FullSize(), Eq(0));
        ASSERT_THAT(frame.Payload().empty(), Eq(true));
    }

    TEST(FrameTest, TestSimpleFrameRssiVerification)
    {
        std::uint8_t buffer[10];
        Frame frame(1, 0xfff, 10, buffer);
        ASSERT_THAT(frame.IsRssiValid(), Eq(true));
        ASSERT_THAT(frame.Verify(), Eq(true));
    }

    TEST(FrameTest, TestSimpleFrameRssiVerificationFailure)
    {
        std::uint8_t buffer[10];
        Frame frame(1, 0xf000, 10, buffer);
        ASSERT_THAT(frame.IsRssiValid(), Eq(false));
        ASSERT_THAT(frame.Verify(), Eq(false));
    }

    TEST(FrameTest, TestSimpleFrameDoppleVerification)
    {
        std::uint8_t buffer[10];
        Frame frame(0xfff, 1, 10, buffer);
        ASSERT_THAT(frame.IsDopplerValid(), Eq(true));
        ASSERT_THAT(frame.Verify(), Eq(true));
    }

    TEST(FrameTest, TestSimpleFrameDopplerVerificationFailure)
    {
        std::uint8_t buffer[10];
        Frame frame(0xf000, 1, 10, buffer);
        ASSERT_THAT(frame.IsDopplerValid(), Eq(false));
        ASSERT_THAT(frame.Verify(), Eq(false));
    }

    TEST(FrameTest, TestPartialFrame)
    {
        std::uint8_t buffer[10];
        Frame frame(0xf000, 1, 16, buffer);
        ASSERT_THAT(frame.Size(), Eq(10));
        ASSERT_THAT(frame.FullSize(), Eq(16));
        ASSERT_THAT(frame.Payload().empty(), Eq(false));
        ASSERT_THAT(frame.Payload().data(), Eq(buffer));
        ASSERT_THAT(frame.Payload().size(), Eq(10));
        ASSERT_THAT(frame.IsComplete(), Eq(false));
        ASSERT_THAT(frame.Verify(), Eq(false));
    }
}
