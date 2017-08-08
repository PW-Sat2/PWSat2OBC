#include <cstring>
#include "base/reader.h"
#include "base/writer.h"
#include "state/comm/MessageState.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

using state::MessageState;
using testing::Eq;
using testing::StrEq;
using namespace std::chrono_literals;

namespace
{
    TEST(MessageStateTest, DefaultState)
    {
        MessageState state;

        ASSERT_THAT(state.Interval(), Eq(5min));
        ASSERT_THAT(state.RepeatCount(), Eq(3));

        auto msg = reinterpret_cast<const char*>(state.Message().data());

        ASSERT_THAT(msg,
            StrEq("We are the Borg. Lower your shields and surrender your ships. We will add your biological and "
                  "technological distinctiveness to our own. Your culture will adapt to service us. Resistance is futile."));
    }

    TEST(MessageStateTest, CustomState)
    {
        const char* msg = "Test";
        auto msgSpan = gsl::make_span(reinterpret_cast<const std::uint8_t*>(msg), 5);

        MessageState state(7min, 4, msgSpan);

        ASSERT_THAT(state.Interval(), Eq(7min));
        ASSERT_THAT(state.RepeatCount(), Eq(4));

        auto msg2 = reinterpret_cast<const char*>(state.Message().data());

        ASSERT_THAT(msg2, StrEq("Test"));
    }

    TEST(MessageStateTest, Serialize)
    {
        const char* msg = "Test";
        auto msgSpan = gsl::make_span(reinterpret_cast<const std::uint8_t*>(msg), 5);

        MessageState state(7min, 4, msgSpan);

        std::array<std::uint8_t, MessageState::Size()> buffer;
        buffer.fill(0);
        Writer w(buffer);

        state.Write(w);

        decltype(buffer) expected;
        expected.fill(0);
        expected[0] = 7;
        expected[1] = 4;
        expected[2] = 84;
        expected[3] = 101;
        expected[4] = 115;
        expected[5] = 116;

        ASSERT_THAT(w.Status(), Eq(true));
        ASSERT_THAT(buffer, Eq(expected));
    }

    TEST(MessageStateTest, Deserialize)
    {
        std::array<std::uint8_t, MessageState::Size()> buffer;
        buffer.fill(0);
        buffer[0] = 7;
        buffer[1] = 4;
        buffer[2] = 84;
        buffer[3] = 101;
        buffer[4] = 115;
        buffer[5] = 116;

        Reader r(buffer);

        MessageState state;
        state.Read(r);

        ASSERT_THAT(r.Status(), Eq(true));
        ASSERT_THAT(state.Interval(), Eq(7min));
        ASSERT_THAT(state.RepeatCount(), Eq(4));

        auto msg2 = reinterpret_cast<const char*>(state.Message().data());

        ASSERT_THAT(msg2, StrEq("Test"));
    }
}
