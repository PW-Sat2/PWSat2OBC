#include <cstdint>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "utils.h"

using std::uint8_t;
using testing::Eq;
namespace
{
    TEST(OnLeaveTest, ShouldBeCalledOnBlockLeaving)
    {
        std::uint8_t counter = 10;

        {
            auto e = OnLeave([&counter]() { counter++; });

            counter *= 2;
        }

        ASSERT_THAT(counter, Eq(21));
    }

    TEST(OnLeaveTest, ShouldNotBeCalledOnBlockLeavingIfOrderedToSkip)
    {
        std::uint8_t counter = 10;

        {
            auto e = OnLeave([&counter]() { counter++; });

            counter *= 2;

            e.Skip();

            counter *= 3;
        }

        ASSERT_THAT(counter, Eq(60));
    }
}
