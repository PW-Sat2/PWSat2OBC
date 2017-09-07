#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "gmock/gmock-more-matchers.h"
#include "base/reader.h"
#include "base/writer.h"
#include "state/sail/SailState.hpp"

using testing::Eq;
using testing::ElementsAre;

TEST(SailStateTest, DefaultState)
{
    state::SailState state;

    ASSERT_THAT(state.CurrentState(), Eq(state::SailOpeningState::Waiting));
    ASSERT_THAT(state.IsDeploymentDisabled(), Eq(false));
}

TEST(SailStateTest, CustomState)
{
    state::SailState state(state::SailOpeningState::OpeningStopped, true);

    ASSERT_THAT(state.CurrentState(), Eq(state::SailOpeningState::OpeningStopped));
    ASSERT_THAT(state.IsDeploymentDisabled(), Eq(true));
}

TEST(SailStateTest, ReadFromBuffer)
{
    state::SailState state;

    std::uint8_t buf[] = {1, 1};
    Reader r(buf);
    state.Read(r);

    ASSERT_THAT(state.CurrentState(), Eq(state::SailOpeningState::Opening));
    ASSERT_THAT(state.IsDeploymentDisabled(), Eq(true));
}

TEST(SailStateTest, WriteToBuffer)
{
    state::SailState state(state::SailOpeningState::OpeningStopped, true);

    std::uint8_t buf[] = {14, 18};
    Writer w(buf);
    state.Write(w);

    ASSERT_THAT(buf, ElementsAre(2, 1));
}
