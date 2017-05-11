#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "base/hertz.hpp"

using testing::Eq;

namespace
{
    class HertzTest : public testing::Test
    {
    };

    TEST_F(HertzTest, ShouldConvertToSeconds)
    {
        auto hertz = chrono_extensions::hertz(0.5);

        auto seconds = chrono_extensions::period_cast<std::chrono::seconds>(hertz);

        ASSERT_THAT(seconds, Eq(std::chrono::seconds(2)));
    }

    TEST_F(HertzTest, ShouldLoadFromLiterals)
    {
        using namespace chrono_extensions::chrono_literals;

        auto hertz = 60_hz;

        ASSERT_THAT(hertz, Eq(chrono_extensions::hertz(60)));
    }
}
