#include <vector>
#include "gtest/gtest.h"
#include "rapidcheck.h"
#include "rapidcheck/gtest.h"

RC_GTEST_PROP(RapidCheck, Test1, (const std::vector<int>& l0))
{
    auto l1 = l0;
    std::reverse(begin(l1), end(l1));
    std::reverse(begin(l1), end(l1));
    RC_ASSERT(l0 == l1);
}

RC_GTEST_PROP(RapidCheck, Test2, (const int a, const int b))
{
    int r1 = a + b;
    int r2 = b + a;

    if (r2 > 5)
    {
        r2++;
    }

    RC_ASSERT(r1 == r2);
}

struct SomeState
{
    int a;
    int b;
    int result;
};

std::ostream& operator<<(std::ostream& os, const SomeState& state)
{
    os << "(a=" << state.a << ", b=" << state.b << ")";
}

namespace rc
{
    template <> struct Arbitrary<SomeState>
    {
        static Gen<SomeState> arbitrary()
        {
            return gen::build<SomeState>(
                gen::set(&SomeState::a, gen::inRange(-20, 20)), gen::set(&SomeState::b, gen::inRange(-20, 20)));
        }
    };
}

void calculate(SomeState* state)
{
    state->result = state->a + state->b;
    if (state->result > 10)
    {
        state->result *= -1;
    }
}

RC_GTEST_PROP(RapidCheck, Test3, (SomeState state))
{
    calculate(&state);

    int expected = state.a + state.b;

    if (expected > 10)
    {
        expected = -expected;
    }

    RC_ASSERT(state.result == expected);
}
