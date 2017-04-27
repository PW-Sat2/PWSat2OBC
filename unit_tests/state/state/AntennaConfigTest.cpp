#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "base/reader.h"
#include "base/writer.h"
#include "state/antenna/AntennaConfiguration.hpp"

using testing::Eq;
namespace
{
    TEST(AntennaConfigurationTest, TestDefaultState)
    {
        state::AntennaConfiguration object;
        ASSERT_THAT(object.IsDeploymentDisabled(), Eq(false));
    }

    TEST(AntennaConfigurationTest, TestNonDefaultState)
    {
        state::AntennaConfiguration object(true);
        ASSERT_THAT(object.IsDeploymentDisabled(), Eq(true));
    }

    TEST(AntennaConfigurationTest, TestStateReading)
    {
        std::uint8_t buffer[] = {0x01};
        Reader reader(buffer);
        state::AntennaConfiguration object;
        object.Read(reader);
        ASSERT_THAT(object.IsDeploymentDisabled(), Eq(true));
    }

    TEST(AntennaConfigurationTest, TestStateWriting)
    {
        std::uint8_t expected[] = {0x01};
        std::uint8_t buffer[32];
        Writer writer(buffer);
        state::AntennaConfiguration object(true);
        object.Write(writer);
        ASSERT_THAT(gsl::make_span(expected), Eq(writer.Capture()));
        ASSERT_THAT(static_cast<std::uint32_t>(writer.Capture().size()), Eq(state::AntennaConfiguration::Size()));
    }
}
