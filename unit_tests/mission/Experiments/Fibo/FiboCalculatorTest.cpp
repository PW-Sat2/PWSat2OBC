#include <array>
#include <cstdint>
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "experiment/fibo/fibo.h"

using testing::ElementsAre;
using experiment::fibo::Fibonacci;
namespace
{
    TEST(FiboCalculatorTest, X)
    {
        Fibonacci f;
        std::array<std::uint32_t, 10> results;

        for (auto& i : results)
        {
            i = f.Current();

            f.Next();
        }

        ASSERT_THAT(results, ElementsAre(1, 1, 2, 3, 5, 8, 13, 21, 34, 55));
    }
}
