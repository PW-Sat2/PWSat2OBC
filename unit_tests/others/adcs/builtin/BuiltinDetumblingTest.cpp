#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "OsMock.hpp"
#include "adcs/BuiltinDetumbling.hpp"
#include "mock/ImtqDriverMock.hpp"
#include "mock/power.hpp"

using testing::_;
using testing::Return;
using testing::Eq;
using testing::Ne;
using testing::Invoke;
using namespace std::chrono_literals;

namespace
{
    class BuiltinDetumblingTest : public testing::Test
    {
      protected:
        testing::NiceMock<ImtqDriverMock> _imtqDriver;
        testing::NiceMock<PowerControlMock> _power;
        testing::NiceMock<OSMock> _os;
        OSReset _osReset{InstallProxy(&_os)};

        adcs::BuiltinDetumbling _detumbling{_imtqDriver, _power};
    };

    TEST_F(BuiltinDetumblingTest, ShouldStartDetumblingDuringProcess)
    {
        EXPECT_CALL(_power, ImtqPower(true)).WillOnce(Return(true));
        EXPECT_CALL(_os, Sleep(1000ms));
        EXPECT_CALL(_imtqDriver, StartBDotDetumbling(_));

        _detumbling.Initialize();
        _detumbling.Enable();
        _detumbling.Process();
    }

    TEST_F(BuiltinDetumblingTest, ShouldFailWhenFailedToPowerUpImtq)
    {
        EXPECT_CALL(_power, ImtqPower(true)).WillOnce(Return(false));

        auto r = _detumbling.Enable();

        ASSERT_THAT(r, Eq(OSResult::IOError));
    }

    TEST_F(BuiltinDetumblingTest, ShouldPowerDownOnDisable)
    {
        EXPECT_CALL(_power, ImtqPower(true)).WillOnce(Return(true));
        EXPECT_CALL(_power, ImtqPower(false)).WillOnce(Return(true));

        _detumbling.Initialize();
        _detumbling.Enable();
        _detumbling.Disable();
    }

    TEST_F(BuiltinDetumblingTest, ShouldPerformSelfTestWhenEnabling)
    {
        EXPECT_CALL(_power, ImtqPower(true)).WillOnce(Return(true));
        EXPECT_CALL(_imtqDriver, PerformSelfTest(_, _)).WillOnce(Return(true));

        _detumbling.Initialize();
        auto r = _detumbling.Enable();
        ASSERT_THAT(r, Eq(OSResult::Success));
    }

    TEST_F(BuiltinDetumblingTest, ShouldEnableIfSelfTestFails)
    {
        EXPECT_CALL(_power, ImtqPower(true)).WillOnce(Return(true));
        EXPECT_CALL(_imtqDriver, PerformSelfTest(_, _)).WillOnce(Return(false));

        _detumbling.Initialize();
        auto r = _detumbling.Enable();
        ASSERT_THAT(r, Eq(OSResult::Success));
    }

    TEST_F(BuiltinDetumblingTest, ShouldEnableIfErrorsInSelfTest)
    {
        EXPECT_CALL(_power, ImtqPower(true)).WillOnce(Return(true));
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
