#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "base/reader.h"
#include "base/writer.h"
#include "state/boot/BootState.hpp"

using testing::Eq;

TEST(BootState, TestDefaultState)
{
    state::BootState boot;
    ASSERT_THAT(boot.BootCounter(), Eq(0u));
}

TEST(BootState, TestCustomState)
{
    state::BootState boot(10);
    ASSERT_THAT(boot.BootCounter(), Eq(10u));
}

TEST(BootState, TestReading)
{
    std::uint8_t buffer[state::BootState::Size()] = {0x55, 0xAA, 0x77, 0xEE};
    Reader reader(buffer);
    state::BootState boot;
    boot.Read(reader);
    ASSERT_THAT(reader.Status(), Eq(true));
    ASSERT_THAT(boot.BootCounter(), Eq(0xEE77AA55));
}

TEST(BootState, TestWriting)
{
    std::uint8_t expected[] = {0xEE, 0x77, 0xAA, 0x55};
    std::uint8_t buffer[state::BootState::Size()];
    Writer writer(buffer);
    state::BootState boot(0x55AA77EE);
    boot.Write(writer);
    ASSERT_THAT(writer.Status(), Eq(true));
    ASSERT_THAT(writer.Capture(), Eq(gsl::make_span(expected)));
}
