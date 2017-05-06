#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "experiment/adcs/adcs.hpp"
#include "mock/AdcsMocks.hpp"
#include "mock/time.hpp"

using testing::NiceMock;
using testing::Return;
using testing::InSequence;
using testing::Eq;

using namespace experiment::adcs;
using namespace adcs;
using experiments::IterationResult;
using experiments::StartResult;
using namespace std::chrono_literals;
namespace
{
    class DetumblingExperimentTest : public testing::Test
    {
      public:
        DetumblingExperimentTest();

      protected:
        NiceMock<AdcsCoordinatorMock> _adcs;
        NiceMock<CurrentTimeMock> _time;
        DetumblingExperiment _exp;
    };

    DetumblingExperimentTest::DetumblingExperimentTest() : _exp(this->_adcs, this->_time)
    {
        ON_CALL(this->_time, GetCurrentTime()).WillByDefault(Return(Some(10ms)));
    }

    TEST_F(DetumblingExperimentTest, ShouldSwitchToExperimentalDetumblingOnStart)
    {
        {
            InSequence s;
            EXPECT_CALL(this->_adcs, Disable()).WillOnce(Return(OSResult::Success));
            EXPECT_CALL(this->_adcs, EnableExperimentalDetumbling()).WillOnce(Return(OSResult::Success));
        }

        auto r = this->_exp.Start();

        ASSERT_THAT(r, Eq(StartResult::Success));
    }

    TEST_F(DetumblingExperimentTest, ShouldSwitchbackToBuiltinDetumblingOnStop)
    {
        {
            InSequence s;
            EXPECT_CALL(this->_adcs, Disable()).WillOnce(Return(OSResult::Success));
            EXPECT_CALL(this->_adcs, EnableBuiltinDetumbling()).WillOnce(Return(OSResult::Success));
        }
        this->_exp.Stop(IterationResult::Finished);
    }

    TEST_F(DetumblingExperimentTest, FailOnDisablingADCSModeWillAbortExperiment)
    {
        {
            InSequence s;
            EXPECT_CALL(this->_adcs, Disable()).WillOnce(Return(OSResult::InvalidOperation));
            EXPECT_CALL(this->_adcs, EnableExperimentalDetumbling()).Times(0);
        }

        auto r = this->_exp.Start();

        ASSERT_THAT(r, Eq(StartResult::Failure));
    }

    TEST_F(DetumblingExperimentTest, FailOnEnablingExperimentalDetumblingWillAbortExperimentAndBringBackBuiltin)
    {
        {
            InSequence s;
            EXPECT_CALL(this->_adcs, Disable()).WillOnce(Return(OSResult::Success));
            EXPECT_CALL(this->_adcs, EnableExperimentalDetumbling()).WillOnce(Return(OSResult::InvalidOperation));
            EXPECT_CALL(this->_adcs, EnableBuiltinDetumbling()).WillOnce(Return(OSResult::Success));
        }

        auto r = this->_exp.Start();

        ASSERT_THAT(r, Eq(StartResult::Failure));
    }

    TEST_F(DetumblingExperimentTest, ShouldNotReenableBuiltinDetumblingOnStopIfDisablingFailed)
    {
        {
            InSequence s;
            EXPECT_CALL(this->_adcs, Disable()).WillOnce(Return(OSResult::InvalidOperation));
            EXPECT_CALL(this->_adcs, EnableBuiltinDetumbling()).Times(0);
        }
        this->_exp.Stop(IterationResult::Finished);
    }
}
