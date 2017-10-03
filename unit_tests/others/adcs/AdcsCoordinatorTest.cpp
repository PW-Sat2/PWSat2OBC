#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "OsMock.hpp"
#include "adcs/AdcsCoordinator.hpp"
#include "mock/AdcsMocks.hpp"

using testing::_;
using testing::Eq;
using testing::Ne;
using testing::Return;
using adcs::AdcsMode;

namespace
{
    class AdcsCoordinatorTest : public testing::Test
    {
      protected:
        AdcsCoordinatorTest();

        std::pair<AdcsMode, bool> EnableBuiltinDetumbling();
        std::pair<AdcsMode, bool> EnableExperimentalDetumbling();
        std::pair<AdcsMode, bool> EnableSunPointing();
        std::pair<AdcsMode, bool> Disable();
        std::pair<AdcsMode, bool> Stop();

        void AssertModeChange(std::pair<AdcsMode, bool> current, AdcsMode expectedMode, bool expectedModeSwitch);
        testing::StrictMock<DetumblingMock> primaryDetumbling;
        testing::StrictMock<DetumblingMock> experimentalDetumbling;
        testing::StrictMock<SunPointingMock> sunPointingAlgorithm;
        testing::NiceMock<OSMock> os;
        OSReset osReset;
        adcs::AdcsCoordinator coordinator;
    };

    AdcsCoordinatorTest::AdcsCoordinatorTest() //
        : coordinator(primaryDetumbling, experimentalDetumbling, sunPointingAlgorithm)
    {
        osReset = InstallProxy(&os);
    }

    std::pair<AdcsMode, bool> AdcsCoordinatorTest::EnableBuiltinDetumbling()
    {
        return coordinator.SwitchMode(AdcsMode::BuiltinDetumbling);
    }

    std::pair<AdcsMode, bool> AdcsCoordinatorTest::EnableExperimentalDetumbling()
    {
        return coordinator.SwitchMode(AdcsMode::ExperimentalDetumbling);
    }

    std::pair<AdcsMode, bool> AdcsCoordinatorTest::EnableSunPointing()
    {
        return coordinator.SwitchMode(AdcsMode::ExperimentalSunpointing);
    }

    std::pair<AdcsMode, bool> AdcsCoordinatorTest::Disable()
    {
        return coordinator.SwitchMode(AdcsMode::Disabled);
    }

    std::pair<AdcsMode, bool> AdcsCoordinatorTest::Stop()
    {
        return coordinator.SwitchMode(AdcsMode::Stopped);
    }

    void AdcsCoordinatorTest::AssertModeChange(std::pair<AdcsMode, bool> current, AdcsMode expectedMode, bool expectedModeSwitch)
    {
        ASSERT_THAT(current.first, Eq(expectedMode));
        ASSERT_THAT(current.second, Eq(expectedModeSwitch));
        ASSERT_THAT(coordinator.CurrentMode(), Eq(expectedMode));
    }

    TEST_F(AdcsCoordinatorTest, EnablePrimaryDetumbling)
    {
        EXPECT_CALL(primaryDetumbling, Enable()).WillOnce(Return(OSResult::Success));
        AssertModeChange(EnableBuiltinDetumbling(), AdcsMode::BuiltinDetumbling, true);
    }

    TEST_F(AdcsCoordinatorTest, EnableExperimentalDetumbling)
    {
        EXPECT_CALL(experimentalDetumbling, Enable()).WillOnce(Return(OSResult::Success));
        AssertModeChange(EnableExperimentalDetumbling(), AdcsMode::ExperimentalDetumbling, true);
    }

    TEST_F(AdcsCoordinatorTest, EnableExperimentalSunPointing)
    {
        EXPECT_CALL(sunPointingAlgorithm, Enable()).WillOnce(Return(OSResult::Success));
        AssertModeChange(EnableSunPointing(), AdcsMode::ExperimentalSunpointing, true);
    }

    TEST_F(AdcsCoordinatorTest, EnablePrimaryDetumblingTwice)
    {
        EXPECT_CALL(primaryDetumbling, Enable()).WillOnce(Return(OSResult::Success));
        EnableBuiltinDetumbling();
        AssertModeChange(EnableBuiltinDetumbling(), AdcsMode::BuiltinDetumbling, false);
    }

    TEST_F(AdcsCoordinatorTest, EnableExperimentalDetumblingTwice)
    {
        EXPECT_CALL(experimentalDetumbling, Enable()).WillOnce(Return(OSResult::Success));
        EnableExperimentalDetumbling();
        AssertModeChange(EnableExperimentalDetumbling(), AdcsMode::ExperimentalDetumbling, false);
    }

    TEST_F(AdcsCoordinatorTest, EnableExperimentalSunPointingTwice)
    {
        EXPECT_CALL(sunPointingAlgorithm, Enable()).WillOnce(Return(OSResult::Success));
        EnableSunPointing();
        AssertModeChange(EnableSunPointing(), AdcsMode::ExperimentalSunpointing, false);
    }

    TEST_F(AdcsCoordinatorTest, DisablePrimaryDetumbling)
    {
        EXPECT_CALL(primaryDetumbling, Enable()).WillOnce(Return(OSResult::Success));
        EnableBuiltinDetumbling();
        EXPECT_CALL(primaryDetumbling, Disable()).WillOnce(Return(OSResult::Success));
        AssertModeChange(Disable(), AdcsMode::Disabled, true);
    }

    TEST_F(AdcsCoordinatorTest, DisableExperimentalDetumbling)
    {
        EXPECT_CALL(experimentalDetumbling, Enable()).WillOnce(Return(OSResult::Success));
        EnableExperimentalDetumbling();
        EXPECT_CALL(experimentalDetumbling, Disable()).WillOnce(Return(OSResult::Success));
        AssertModeChange(Disable(), AdcsMode::Disabled, true);
    }

    TEST_F(AdcsCoordinatorTest, DisableExperimentalSunPointing)
    {
        EXPECT_CALL(sunPointingAlgorithm, Enable()).WillOnce(Return(OSResult::Success));
        EnableSunPointing();
        EXPECT_CALL(sunPointingAlgorithm, Disable()).WillOnce(Return(OSResult::Success));
        AssertModeChange(Disable(), AdcsMode::Disabled, true);
    }

    TEST_F(AdcsCoordinatorTest, StopPrimaryDetumbling)
    {
        EXPECT_CALL(primaryDetumbling, Enable()).WillOnce(Return(OSResult::Success));
        EnableBuiltinDetumbling();
        EXPECT_CALL(primaryDetumbling, Disable()).WillOnce(Return(OSResult::Success));
        AssertModeChange(Stop(), AdcsMode::Stopped, true);
    }

    TEST_F(AdcsCoordinatorTest, StopExperimentalDetumbling)
    {
        EXPECT_CALL(experimentalDetumbling, Enable()).WillOnce(Return(OSResult::Success));
        EnableExperimentalDetumbling();
        EXPECT_CALL(experimentalDetumbling, Disable()).WillOnce(Return(OSResult::Success));
        AssertModeChange(Stop(), AdcsMode::Stopped, true);
    }

    TEST_F(AdcsCoordinatorTest, StopExperimentalSunPointing)
    {
        EXPECT_CALL(sunPointingAlgorithm, Enable()).WillOnce(Return(OSResult::Success));
        EnableSunPointing();
        EXPECT_CALL(sunPointingAlgorithm, Disable()).WillOnce(Return(OSResult::Success));
        AssertModeChange(Stop(), AdcsMode::Stopped, true);
    }

    TEST_F(AdcsCoordinatorTest, DisableInStoppedMode)
    {
        AssertModeChange(Disable(), AdcsMode::Disabled, true);
    }

    TEST_F(AdcsCoordinatorTest, StopDisabledMode)
    {
        Disable();
        AssertModeChange(Stop(), AdcsMode::Stopped, true);
    }

    TEST_F(AdcsCoordinatorTest, EnablePrimaryDetumblingInDisabledMode)
    {
        Disable();
        EXPECT_CALL(primaryDetumbling, Enable()).Times(0);
        EXPECT_CALL(primaryDetumbling, Disable()).Times(0);
        AssertModeChange(EnableBuiltinDetumbling(), AdcsMode::Disabled, false);
    }

    TEST_F(AdcsCoordinatorTest, EnableExperimentalDetumblingInDisabledMode)
    {
        Disable();
        EXPECT_CALL(experimentalDetumbling, Enable()).Times(0);
        EXPECT_CALL(experimentalDetumbling, Disable()).Times(0);
        AssertModeChange(EnableExperimentalDetumbling(), AdcsMode::Disabled, false);
    }

    TEST_F(AdcsCoordinatorTest, EnableExperimentalSunPointingInDisabledMode)
    {
        Disable();
        EXPECT_CALL(sunPointingAlgorithm, Enable()).Times(0);
        EXPECT_CALL(sunPointingAlgorithm, Disable()).Times(0);
        AssertModeChange(EnableSunPointing(), AdcsMode::Disabled, false);
    }

    TEST_F(AdcsCoordinatorTest, EnablePrimaryDetumblingInBlockedMode)
    {
        coordinator.SetBlockMode(AdcsMode::BuiltinDetumbling, true);
        EXPECT_CALL(primaryDetumbling, Enable()).Times(0);
        EXPECT_CALL(primaryDetumbling, Disable()).Times(0);
        AssertModeChange(EnableBuiltinDetumbling(), AdcsMode::Stopped, false);
    }

    TEST_F(AdcsCoordinatorTest, EnableExperimentalDetumblingInBlockedMode)
    {
        coordinator.SetBlockMode(AdcsMode::ExperimentalDetumbling, true);
        EXPECT_CALL(experimentalDetumbling, Enable()).Times(0);
        EXPECT_CALL(experimentalDetumbling, Disable()).Times(0);
        AssertModeChange(EnableExperimentalDetumbling(), AdcsMode::Stopped, false);
    }

    TEST_F(AdcsCoordinatorTest, EnableExperimentalSunPointingInBlockedMode)
    {
        coordinator.SetBlockMode(AdcsMode::ExperimentalSunpointing, true);
        EXPECT_CALL(sunPointingAlgorithm, Enable()).Times(0);
        EXPECT_CALL(sunPointingAlgorithm, Disable()).Times(0);
        AssertModeChange(EnableSunPointing(), AdcsMode::Stopped, false);
    }

    TEST_F(AdcsCoordinatorTest, EnablePrimaryDetumblingFailure)
    {
        EXPECT_CALL(primaryDetumbling, Enable()).WillOnce(Return(OSResult::IOError));
        AssertModeChange(EnableBuiltinDetumbling(), AdcsMode::Stopped, false);
    }

    TEST_F(AdcsCoordinatorTest, EnableExperimentalDetumblingFailure)
    {
        EXPECT_CALL(experimentalDetumbling, Enable()).WillOnce(Return(OSResult::IOError));
        AssertModeChange(EnableExperimentalDetumbling(), AdcsMode::Stopped, false);
    }

    TEST_F(AdcsCoordinatorTest, EnableExperimentalSunPointingFailure)
    {
        EXPECT_CALL(sunPointingAlgorithm, Enable()).WillOnce(Return(OSResult::IOError));
        AssertModeChange(EnableSunPointing(), AdcsMode::Stopped, false);
    }

    TEST_F(AdcsCoordinatorTest, DisablePrimaryDetumblingFailure)
    {
        EXPECT_CALL(primaryDetumbling, Enable()).WillOnce(Return(OSResult::Success));
        EnableBuiltinDetumbling();
        EXPECT_CALL(primaryDetumbling, Disable()).WillOnce(Return(OSResult::IOError));
        AssertModeChange(Disable(), AdcsMode::Stopped, true);
    }

    TEST_F(AdcsCoordinatorTest, DisableExperimentalDetumblingFailure)
    {
        EXPECT_CALL(experimentalDetumbling, Enable()).WillOnce(Return(OSResult::Success));
        EnableExperimentalDetumbling();
        EXPECT_CALL(experimentalDetumbling, Disable()).WillOnce(Return(OSResult::IOError));
        AssertModeChange(Disable(), AdcsMode::Stopped, true);
    }

    TEST_F(AdcsCoordinatorTest, DisableExperimentalSunPointingFailure)
    {
        EXPECT_CALL(sunPointingAlgorithm, Enable()).WillOnce(Return(OSResult::Success));
        EnableSunPointing();
        EXPECT_CALL(sunPointingAlgorithm, Disable()).WillOnce(Return(OSResult::IOError));
        AssertModeChange(Disable(), AdcsMode::Stopped, true);
    }

    TEST_F(AdcsCoordinatorTest, TestSwitchingDetumblingMode)
    {
        EXPECT_CALL(experimentalDetumbling, Enable()).WillOnce(Return(OSResult::Success));
        EnableExperimentalDetumbling();
        EXPECT_CALL(experimentalDetumbling, Disable()).WillOnce(Return(OSResult::Success));
        EXPECT_CALL(primaryDetumbling, Enable()).WillOnce(Return(OSResult::Success));
        AssertModeChange(EnableBuiltinDetumbling(), AdcsMode::BuiltinDetumbling, true);
    }

    TEST_F(AdcsCoordinatorTest, TestSwitchingDetumblingFailure)
    {
        EXPECT_CALL(experimentalDetumbling, Enable()).WillOnce(Return(OSResult::Success));
        EnableExperimentalDetumbling();
        EXPECT_CALL(experimentalDetumbling, Disable()).WillOnce(Return(OSResult::Success));
        EXPECT_CALL(primaryDetumbling, Enable()).WillOnce(Return(OSResult::IOError));
        AssertModeChange(EnableBuiltinDetumbling(), AdcsMode::Stopped, true);
    }

    TEST_F(AdcsCoordinatorTest, TestSwitchingDetumblingFailureOnDisable)
    {
        EXPECT_CALL(experimentalDetumbling, Enable()).WillOnce(Return(OSResult::Success));
        EnableExperimentalDetumbling();
        EXPECT_CALL(experimentalDetumbling, Disable()).WillOnce(Return(OSResult::IOError));
        EXPECT_CALL(primaryDetumbling, Enable()).Times(0);
        AssertModeChange(EnableBuiltinDetumbling(), AdcsMode::Stopped, true);
    }

    TEST_F(AdcsCoordinatorTest, TestSwitchingDetumblingToSunPointing)
    {
        EXPECT_CALL(experimentalDetumbling, Enable()).WillOnce(Return(OSResult::Success));
        EnableExperimentalDetumbling();
        EXPECT_CALL(experimentalDetumbling, Disable()).WillOnce(Return(OSResult::Success));
        EXPECT_CALL(sunPointingAlgorithm, Enable()).WillOnce(Return(OSResult::Success));
        AssertModeChange(EnableSunPointing(), AdcsMode::ExperimentalSunpointing, true);
    }

    TEST_F(AdcsCoordinatorTest, TestSwitchingDetumblingToSunPointingFailure)
    {
        EXPECT_CALL(experimentalDetumbling, Enable()).WillOnce(Return(OSResult::Success));
        EnableExperimentalDetumbling();
        EXPECT_CALL(experimentalDetumbling, Disable()).WillOnce(Return(OSResult::Success));
        EXPECT_CALL(sunPointingAlgorithm, Enable()).WillOnce(Return(OSResult::IOError));
        AssertModeChange(EnableSunPointing(), AdcsMode::Stopped, true);
    }

    TEST_F(AdcsCoordinatorTest, TestSwitchingDetumblingToSunPointingFailureOnDisable)
    {
        EXPECT_CALL(experimentalDetumbling, Enable()).WillOnce(Return(OSResult::Success));
        EnableExperimentalDetumbling();
        EXPECT_CALL(experimentalDetumbling, Disable()).WillOnce(Return(OSResult::IOError));
        EXPECT_CALL(sunPointingAlgorithm, Enable()).Times(0);
        AssertModeChange(EnableSunPointing(), AdcsMode::Stopped, true);
    }

    TEST_F(AdcsCoordinatorTest, TestSwitchingSunPointingToPrimaryDetumbling)
    {
        EXPECT_CALL(sunPointingAlgorithm, Enable()).WillOnce(Return(OSResult::Success));
        EnableSunPointing();
        EXPECT_CALL(sunPointingAlgorithm, Disable()).WillOnce(Return(OSResult::Success));
        EXPECT_CALL(primaryDetumbling, Enable()).WillOnce(Return(OSResult::Success));
        AssertModeChange(EnableBuiltinDetumbling(), AdcsMode::BuiltinDetumbling, true);
    }

    TEST_F(AdcsCoordinatorTest, TestSwitchingSunPointingToPrimaryDetumblingFailure)
    {
        EXPECT_CALL(sunPointingAlgorithm, Enable()).WillOnce(Return(OSResult::Success));
        EnableSunPointing();
        EXPECT_CALL(sunPointingAlgorithm, Disable()).WillOnce(Return(OSResult::Success));
        EXPECT_CALL(primaryDetumbling, Enable()).WillOnce(Return(OSResult::IOError));
        AssertModeChange(EnableBuiltinDetumbling(), AdcsMode::Stopped, true);
    }

    TEST_F(AdcsCoordinatorTest, TestSwitchingSunPointingToPrimaryDetumblingFailureOnDisable)
    {
        EXPECT_CALL(sunPointingAlgorithm, Enable()).WillOnce(Return(OSResult::Success));
        EnableSunPointing();
        EXPECT_CALL(sunPointingAlgorithm, Disable()).WillOnce(Return(OSResult::IOError));
        EXPECT_CALL(primaryDetumbling, Enable()).Times(0);
        AssertModeChange(EnableBuiltinDetumbling(), AdcsMode::Stopped, true);
    }

    TEST_F(AdcsCoordinatorTest, TestModeBlockDefaultState)
    {
        ASSERT_THAT(coordinator.IsModeBlocked(AdcsMode::BuiltinDetumbling), Eq(false));
        ASSERT_THAT(coordinator.IsModeBlocked(AdcsMode::ExperimentalDetumbling), Eq(false));
        ASSERT_THAT(coordinator.IsModeBlocked(AdcsMode::ExperimentalSunpointing), Eq(false));
    }

    TEST_F(AdcsCoordinatorTest, TestModeBlockOutOfRange)
    {
        ASSERT_THAT(coordinator.IsModeBlocked(AdcsMode::Disabled), Eq(true));
        ASSERT_THAT(coordinator.IsModeBlocked(static_cast<AdcsMode>(0xff)), Eq(true));
    }

    TEST_F(AdcsCoordinatorTest, TestBlockingMode)
    {
        coordinator.SetBlockMode(AdcsMode::BuiltinDetumbling, true);
        ASSERT_THAT(coordinator.IsModeBlocked(AdcsMode::BuiltinDetumbling), Eq(true));
        ASSERT_THAT(coordinator.IsModeBlocked(AdcsMode::ExperimentalDetumbling), Eq(false));
        ASSERT_THAT(coordinator.IsModeBlocked(AdcsMode::ExperimentalSunpointing), Eq(false));
    }

    TEST_F(AdcsCoordinatorTest, TestUnblockingMode)
    {
        coordinator.SetBlockMode(AdcsMode::BuiltinDetumbling, true);
        coordinator.SetBlockMode(AdcsMode::BuiltinDetumbling, false);
        ASSERT_THAT(coordinator.IsModeBlocked(AdcsMode::BuiltinDetumbling), Eq(false));
        ASSERT_THAT(coordinator.IsModeBlocked(AdcsMode::ExperimentalDetumbling), Eq(false));
        ASSERT_THAT(coordinator.IsModeBlocked(AdcsMode::ExperimentalSunpointing), Eq(false));
    }

    TEST_F(AdcsCoordinatorTest, TestUnblockingDisabledMode)
    {
        coordinator.SetBlockMode(AdcsMode::Disabled, false);
        ASSERT_THAT(coordinator.IsModeBlocked(AdcsMode::Disabled), Eq(true));
    }
}
