#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "adcs/BuiltinDetumbling.hpp"
#include "mock/ImtqDriverMock.hpp"

using testing::_;
using testing::Return;
using testing::Eq;
using testing::Ne;
using testing::Invoke;

namespace
{
    class BuiltinDetumblingTest : public testing::Test
    {
      protected:
        testing::NiceMock<ImtqDriverMock> _imtqDriver;

        adcs::BuiltinDetumbling _detumbling{_imtqDriver};
    };

    TEST_F(BuiltinDetumblingTest, ShouldStartDetumblingDuringProcess)
    {
        EXPECT_CALL(_imtqDriver, StartBDotDetumbling(_));

        _detumbling.Initialize();
        _detumbling.Enable();
        _detumbling.Process();
    }

    TEST_F(BuiltinDetumblingTest, ShouldCancelDetumblingOnDisable)
    {
        EXPECT_CALL(_imtqDriver, CancelOperation());

        _detumbling.Initialize();
        _detumbling.Enable();
        _detumbling.Disable();
    }

    TEST_F(BuiltinDetumblingTest, ShouldPerformSelfTestWhenEnabling)
    {
        EXPECT_CALL(_imtqDriver, PerformSelfTest(_, _)).WillOnce(Return(true));

        _detumbling.Initialize();
        auto r = _detumbling.Enable();
        ASSERT_THAT(r, Eq(OSResult::Success));
    }

    TEST_F(BuiltinDetumblingTest, ShouldFailToEnableIfSelfTestFails)
    {
        EXPECT_CALL(_imtqDriver, PerformSelfTest(_, _)).WillOnce(Return(false));

        _detumbling.Initialize();
        auto r = _detumbling.Enable();
        ASSERT_THAT(r, Eq(OSResult::Success));
    }

    TEST_F(BuiltinDetumblingTest, ShouldFailToEnableIfErrorsInSelfTest)
    {
        EXPECT_CALL(_imtqDriver, PerformSelfTest(_, _)).WillOnce(Invoke([](devices::imtq::SelfTestResult& result, bool) {
            for (auto& s : result.stepResults)
            {
                s.error = devices::imtq::Error(5);
            }
            return true;
        }));

        _detumbling.Initialize();
        auto r = _detumbling.Enable();
        ASSERT_THAT(r, Eq(OSResult::Success));
    }
}
