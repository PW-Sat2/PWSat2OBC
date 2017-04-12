#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "adcs/AdcsCoordinator.hpp"
#include "mock/AdcsMocks.hpp"

using testing::Eq;
using testing::Ne;
using testing::Return;
using adcs::AdcsMode;

class AdcsCoordinatorTest : public testing::Test
{
  protected:
    AdcsCoordinatorTest();

    testing::StrictMock<DetumblingMock> primaryDetumling;
    testing::StrictMock<DetumblingMock> experimentalDetumbling;
    testing::StrictMock<SunPointingMock> sunPointingAlgorithm;
    adcs::AdcsCoordinator coordinator;
};

AdcsCoordinatorTest::AdcsCoordinatorTest() //
    : coordinator(primaryDetumling, experimentalDetumbling, sunPointingAlgorithm)
{
}

TEST_F(AdcsCoordinatorTest, EnablePrimaryDetumbling)
{
    EXPECT_CALL(primaryDetumling, EnableDetumbling()).WillOnce(Return(OSResult::Success));
    const auto result = coordinator.EnableBuiltinDetumbling();
    ASSERT_THAT(result, Eq(OSResult::Success));
    ASSERT_THAT(coordinator.CurrentMode(), Eq(AdcsMode::BuiltinDetumbling));
}

TEST_F(AdcsCoordinatorTest, EnableExperimentalDetumbling)
{
    EXPECT_CALL(experimentalDetumbling, EnableDetumbling()).WillOnce(Return(OSResult::Success));
    const auto result = coordinator.EnableCustomDetumbling();
    ASSERT_THAT(result, Eq(OSResult::Success));
    ASSERT_THAT(coordinator.CurrentMode(), Eq(AdcsMode::CustomDetumbling));
}

TEST_F(AdcsCoordinatorTest, EnableExperimentalSunPointing)
{
    EXPECT_CALL(sunPointingAlgorithm, EnableSunPointing()).WillOnce(Return(OSResult::Success));
    const auto result = coordinator.EnableSunPointing();
    ASSERT_THAT(result, Eq(OSResult::Success));
    ASSERT_THAT(coordinator.CurrentMode(), Eq(AdcsMode::CustomSunpointing));
}

TEST_F(AdcsCoordinatorTest, EnablePrimaryDetumblingTwice)
{
    EXPECT_CALL(primaryDetumling, EnableDetumbling()).WillOnce(Return(OSResult::Success));
    coordinator.EnableBuiltinDetumbling();
    const auto result = coordinator.EnableBuiltinDetumbling();
    ASSERT_THAT(result, Eq(OSResult::Success));
    ASSERT_THAT(coordinator.CurrentMode(), Eq(AdcsMode::BuiltinDetumbling));
}

TEST_F(AdcsCoordinatorTest, EnableExperimentalDetumblingTwice)
{
    EXPECT_CALL(experimentalDetumbling, EnableDetumbling()).WillOnce(Return(OSResult::Success));
    coordinator.EnableCustomDetumbling();
    const auto result = coordinator.EnableCustomDetumbling();
    ASSERT_THAT(result, Eq(OSResult::Success));
    ASSERT_THAT(coordinator.CurrentMode(), Eq(AdcsMode::CustomDetumbling));
}

TEST_F(AdcsCoordinatorTest, EnableExperimentalSunPointingTwice)
{
    EXPECT_CALL(sunPointingAlgorithm, EnableSunPointing()).WillOnce(Return(OSResult::Success));
    coordinator.EnableSunPointing();
    const auto result = coordinator.EnableSunPointing();
    ASSERT_THAT(result, Eq(OSResult::Success));
    ASSERT_THAT(coordinator.CurrentMode(), Eq(AdcsMode::CustomSunpointing));
}

TEST_F(AdcsCoordinatorTest, DisablePrimaryDetumbling)
{
    EXPECT_CALL(primaryDetumling, EnableDetumbling()).WillOnce(Return(OSResult::Success));
    coordinator.EnableBuiltinDetumbling();
    EXPECT_CALL(primaryDetumling, DisableDetumbling()).WillOnce(Return(OSResult::Success));
    const auto result = coordinator.Disable();
    ASSERT_THAT(result, Eq(OSResult::Success));
    ASSERT_THAT(coordinator.CurrentMode(), Eq(AdcsMode::Disabled));
}

TEST_F(AdcsCoordinatorTest, DisableExperimentalDetumbling)
{
    EXPECT_CALL(experimentalDetumbling, EnableDetumbling()).WillOnce(Return(OSResult::Success));
    coordinator.EnableCustomDetumbling();
    EXPECT_CALL(experimentalDetumbling, DisableDetumbling()).WillOnce(Return(OSResult::Success));
    const auto result = coordinator.Disable();
    ASSERT_THAT(result, Eq(OSResult::Success));
    ASSERT_THAT(coordinator.CurrentMode(), Eq(AdcsMode::Disabled));
}

TEST_F(AdcsCoordinatorTest, DisableExperimentalSunPointing)
{
    EXPECT_CALL(sunPointingAlgorithm, EnableSunPointing()).WillOnce(Return(OSResult::Success));
    coordinator.EnableSunPointing();
    EXPECT_CALL(sunPointingAlgorithm, DisableSunPointing()).WillOnce(Return(OSResult::Success));
    const auto result = coordinator.Disable();
    ASSERT_THAT(result, Eq(OSResult::Success));
    ASSERT_THAT(coordinator.CurrentMode(), Eq(AdcsMode::Disabled));
}

TEST_F(AdcsCoordinatorTest, EnablePrimaryDetumblingFailure)
{
    EXPECT_CALL(primaryDetumling, EnableDetumbling()).WillOnce(Return(OSResult::IOError));
    const auto result = coordinator.EnableBuiltinDetumbling();
    ASSERT_THAT(result, Ne(OSResult::Success));
    ASSERT_THAT(coordinator.CurrentMode(), Eq(AdcsMode::Disabled));
}

TEST_F(AdcsCoordinatorTest, EnableExperimentalDetumblingFailure)
{
    EXPECT_CALL(experimentalDetumbling, EnableDetumbling()).WillOnce(Return(OSResult::IOError));
    const auto result = coordinator.EnableCustomDetumbling();
    ASSERT_THAT(result, Ne(OSResult::Success));
    ASSERT_THAT(coordinator.CurrentMode(), Eq(AdcsMode::Disabled));
}

TEST_F(AdcsCoordinatorTest, EnableExperimentalSunPointingFailure)
{
    EXPECT_CALL(sunPointingAlgorithm, EnableSunPointing()).WillOnce(Return(OSResult::IOError));
    const auto result = coordinator.EnableSunPointing();
    ASSERT_THAT(result, Ne(OSResult::Success));
    ASSERT_THAT(coordinator.CurrentMode(), Eq(AdcsMode::Disabled));
}

TEST_F(AdcsCoordinatorTest, DisablePrimaryDetumblingFailure)
{
    EXPECT_CALL(primaryDetumling, EnableDetumbling()).WillOnce(Return(OSResult::Success));
    coordinator.EnableBuiltinDetumbling();
    EXPECT_CALL(primaryDetumling, DisableDetumbling()).WillOnce(Return(OSResult::IOError));
    const auto result = coordinator.Disable();
    ASSERT_THAT(result, Ne(OSResult::Success));
    ASSERT_THAT(coordinator.CurrentMode(), Eq(AdcsMode::BuiltinDetumbling));
}

TEST_F(AdcsCoordinatorTest, DisableExperimentalDetumblingFailure)
{
    EXPECT_CALL(experimentalDetumbling, EnableDetumbling()).WillOnce(Return(OSResult::Success));
    coordinator.EnableCustomDetumbling();
    EXPECT_CALL(experimentalDetumbling, DisableDetumbling()).WillOnce(Return(OSResult::IOError));
    const auto result = coordinator.Disable();
    ASSERT_THAT(result, Ne(OSResult::Success));
    ASSERT_THAT(coordinator.CurrentMode(), Eq(AdcsMode::CustomDetumbling));
}

TEST_F(AdcsCoordinatorTest, DisableExperimentalSunPointingFailure)
{
    EXPECT_CALL(sunPointingAlgorithm, EnableSunPointing()).WillOnce(Return(OSResult::Success));
    coordinator.EnableSunPointing();
    EXPECT_CALL(sunPointingAlgorithm, DisableSunPointing()).WillOnce(Return(OSResult::IOError));
    const auto result = coordinator.Disable();
    ASSERT_THAT(result, Ne(OSResult::Success));
    ASSERT_THAT(coordinator.CurrentMode(), Eq(AdcsMode::CustomSunpointing));
}

TEST_F(AdcsCoordinatorTest, TestSwitchingDetumblingMode)
{
    EXPECT_CALL(experimentalDetumbling, EnableDetumbling()).WillOnce(Return(OSResult::Success));
    coordinator.EnableCustomDetumbling();
    EXPECT_CALL(experimentalDetumbling, DisableDetumbling()).WillOnce(Return(OSResult::Success));
    EXPECT_CALL(primaryDetumling, EnableDetumbling()).WillOnce(Return(OSResult::Success));
    const auto result = coordinator.EnableBuiltinDetumbling();
    ASSERT_THAT(result, Eq(OSResult::Success));
    ASSERT_THAT(coordinator.CurrentMode(), Eq(AdcsMode::BuiltinDetumbling));
}

TEST_F(AdcsCoordinatorTest, TestSwitchingDetumblingFailure)
{
    EXPECT_CALL(experimentalDetumbling, EnableDetumbling()).WillOnce(Return(OSResult::Success));
    coordinator.EnableCustomDetumbling();
    EXPECT_CALL(experimentalDetumbling, DisableDetumbling()).WillOnce(Return(OSResult::Success));
    EXPECT_CALL(primaryDetumling, EnableDetumbling()).WillOnce(Return(OSResult::IOError));
    const auto result = coordinator.EnableBuiltinDetumbling();
    ASSERT_THAT(result, Ne(OSResult::Success));
    ASSERT_THAT(coordinator.CurrentMode(), Eq(AdcsMode::Disabled));
}

TEST_F(AdcsCoordinatorTest, TestSwitchingDetumblingFailureOnDisable)
{
    EXPECT_CALL(experimentalDetumbling, EnableDetumbling()).WillOnce(Return(OSResult::Success));
    coordinator.EnableCustomDetumbling();
    EXPECT_CALL(experimentalDetumbling, DisableDetumbling()).WillOnce(Return(OSResult::IOError));
    EXPECT_CALL(primaryDetumling, EnableDetumbling()).Times(0);
    const auto result = coordinator.EnableBuiltinDetumbling();
    ASSERT_THAT(result, Ne(OSResult::Success));
    ASSERT_THAT(coordinator.CurrentMode(), Eq(AdcsMode::CustomDetumbling));
}

TEST_F(AdcsCoordinatorTest, TestSwitchingDetumblingToSunPointing)
{
    EXPECT_CALL(experimentalDetumbling, EnableDetumbling()).WillOnce(Return(OSResult::Success));
    coordinator.EnableCustomDetumbling();
    EXPECT_CALL(experimentalDetumbling, DisableDetumbling()).WillOnce(Return(OSResult::Success));
    EXPECT_CALL(sunPointingAlgorithm, EnableSunPointing()).WillOnce(Return(OSResult::Success));
    const auto result = coordinator.EnableSunPointing();
    ASSERT_THAT(result, Eq(OSResult::Success));
    ASSERT_THAT(coordinator.CurrentMode(), Eq(AdcsMode::CustomSunpointing));
}

TEST_F(AdcsCoordinatorTest, TestSwitchingDetumblingToSunPointingFailure)
{
    EXPECT_CALL(experimentalDetumbling, EnableDetumbling()).WillOnce(Return(OSResult::Success));
    coordinator.EnableCustomDetumbling();
    EXPECT_CALL(experimentalDetumbling, DisableDetumbling()).WillOnce(Return(OSResult::Success));
    EXPECT_CALL(sunPointingAlgorithm, EnableSunPointing()).WillOnce(Return(OSResult::IOError));
    const auto result = coordinator.EnableSunPointing();
    ASSERT_THAT(result, Ne(OSResult::Success));
    ASSERT_THAT(coordinator.CurrentMode(), Eq(AdcsMode::Disabled));
}

TEST_F(AdcsCoordinatorTest, TestSwitchingDetumblingToSunPointingFailureOnDisable)
{
    EXPECT_CALL(experimentalDetumbling, EnableDetumbling()).WillOnce(Return(OSResult::Success));
    coordinator.EnableCustomDetumbling();
    EXPECT_CALL(experimentalDetumbling, DisableDetumbling()).WillOnce(Return(OSResult::IOError));
    EXPECT_CALL(sunPointingAlgorithm, EnableSunPointing()).Times(0);
    const auto result = coordinator.EnableSunPointing();
    ASSERT_THAT(result, Ne(OSResult::Success));
    ASSERT_THAT(coordinator.CurrentMode(), Eq(AdcsMode::CustomDetumbling));
}

TEST_F(AdcsCoordinatorTest, TestSwitchingSunPointingToPrimaryDetumbling)
{
    EXPECT_CALL(sunPointingAlgorithm, EnableSunPointing()).WillOnce(Return(OSResult::Success));
    coordinator.EnableSunPointing();
    EXPECT_CALL(sunPointingAlgorithm, DisableSunPointing()).WillOnce(Return(OSResult::Success));
    EXPECT_CALL(primaryDetumling, EnableDetumbling()).WillOnce(Return(OSResult::Success));
    const auto result = coordinator.EnableBuiltinDetumbling();
    ASSERT_THAT(result, Eq(OSResult::Success));
    ASSERT_THAT(coordinator.CurrentMode(), Eq(AdcsMode::BuiltinDetumbling));
}

TEST_F(AdcsCoordinatorTest, TestSwitchingSunPointingToPrimaryDetumblingFailure)
{
    EXPECT_CALL(sunPointingAlgorithm, EnableSunPointing()).WillOnce(Return(OSResult::Success));
    coordinator.EnableSunPointing();
    EXPECT_CALL(sunPointingAlgorithm, DisableSunPointing()).WillOnce(Return(OSResult::Success));
    EXPECT_CALL(primaryDetumling, EnableDetumbling()).WillOnce(Return(OSResult::IOError));
    const auto result = coordinator.EnableBuiltinDetumbling();
    ASSERT_THAT(result, Ne(OSResult::Success));
    ASSERT_THAT(coordinator.CurrentMode(), Eq(AdcsMode::Disabled));
}

TEST_F(AdcsCoordinatorTest, TestSwitchingSunPointingToPrimaryDetumblingFailureOnDisable)
{
    EXPECT_CALL(sunPointingAlgorithm, EnableSunPointing()).WillOnce(Return(OSResult::Success));
    coordinator.EnableSunPointing();
    EXPECT_CALL(sunPointingAlgorithm, DisableSunPointing()).WillOnce(Return(OSResult::IOError));
    EXPECT_CALL(primaryDetumling, EnableDetumbling()).Times(0);
    const auto result = coordinator.EnableBuiltinDetumbling();
    ASSERT_THAT(result, Ne(OSResult::Success));
    ASSERT_THAT(coordinator.CurrentMode(), Eq(AdcsMode::CustomSunpointing));
}
