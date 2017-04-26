#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "base/reader.h"
#include "base/writer.h"
#include "state/Time/TimeCorrectionConfiguration.hpp"
#include "state/Time/TimeState.hpp"

using namespace std::chrono_literals;
using testing::Eq;

TEST(TimeStateTest, TestDefaultState)
{
    state::TimeState timeState;
    ASSERT_THAT(timeState.LastExternalTime(), Eq(0ms));
    ASSERT_THAT(timeState.LastMissionTime(), Eq(0ms));
}

TEST(TimeStateTest, TestNonDefaultState)
{
    state::TimeState timeState(10ms, 20ms);
    ASSERT_THAT(timeState.LastExternalTime(), Eq(20ms));
    ASSERT_THAT(timeState.LastMissionTime(), Eq(10ms));
}

TEST(TimeStateTest, TestStateReading)
{
    std::uint8_t buffer[] = {0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x7f, 0x90, 0xa0, 0xb0, 0xc0, 0xd0, 0xe0, 0xef, 0x00};
    Reader reader(buffer);
    state::TimeState timeState;
    timeState.Read(reader);
    ASSERT_THAT(timeState.LastMissionTime(), Eq(0x7f70605040302010ms));
    ASSERT_THAT(timeState.LastExternalTime(), Eq(0x00efe0d0c0b0a090ms));
}

TEST(TimeStateTest, TestStateWriting)
{
    std::uint8_t expected[] = {0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x7f, 0x90, 0xa0, 0xb0, 0xc0, 0xd0, 0xe0, 0xef, 0x00};
    std::uint8_t buffer[32];
    Writer writer(buffer);
    state::TimeState timeState(0x7f70605040302010ms, 0x00efe0d0c0b0a090ms);
    timeState.Write(writer);
    ASSERT_THAT(gsl::make_span(expected), Eq(writer.Capture()));
    ASSERT_THAT(static_cast<std::uint32_t>(writer.Capture().size()), Eq(state::TimeState::Size()));
}

TEST(TimeCorrectionConfiguration, TestDefaultState)
{
    state::TimeCorrectionConfiguration config;
    ASSERT_THAT(config.ExternalTimeFactor(), Eq(1));
    ASSERT_THAT(config.MissionTimeFactor(), Eq(1));
    ASSERT_THAT(config.Total(), Eq(2));
}

TEST(TimeCorrectionConfiguration, TestCustomState)
{
    state::TimeCorrectionConfiguration config(4, 5);
    ASSERT_THAT(config.ExternalTimeFactor(), Eq(5));
    ASSERT_THAT(config.MissionTimeFactor(), Eq(4));
    ASSERT_THAT(config.Total(), Eq(9));
}

TEST(TimeCorrectionConfiguration, TestStateReading)
{
    std::uint8_t buffer[] = {0x10, 0x20, 0x30, 0x40};
    Reader reader(buffer);
    state::TimeCorrectionConfiguration config;
    config.Read(reader);
    ASSERT_THAT(config.MissionTimeFactor(), Eq(0x2010));
    ASSERT_THAT(config.ExternalTimeFactor(), Eq(0x4030));
    ASSERT_THAT(config.Total(), Eq(0x2010 + 0x4030));
}

TEST(TimeCorrectionConfiguration, TestStateWriting)
{
    std::uint8_t expected[] = {0x10, 0x20, 0x30, 0x40};
    std::uint8_t buffer[32];
    Writer writer(buffer);
    state::TimeCorrectionConfiguration config(0x2010, 0x4030);
    config.Write(writer);
    ASSERT_THAT(gsl::make_span(expected), Eq(writer.Capture()));
    ASSERT_THAT(static_cast<std::uint32_t>(writer.Capture().size()), Eq(state::TimeCorrectionConfiguration::Size()));
}
