#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "OsMock.hpp"
#include "adcs/ExperimentalDetumbling.hpp"
#include "mock/ImtqDriverMock.hpp"
#include "mock/power.hpp"

using namespace std::chrono_literals;

using testing::Eq;
using testing::_;
using testing::Return;
using testing::DoAll;
using testing::SetArgReferee;

using devices::imtq::Current;
using devices::imtq::DetumbleData;
using devices::imtq::Dipole;
using devices::imtq::Error;
using devices::imtq::MagnetometerMeasurement;
using devices::imtq::SelfTestResult;
using devices::imtq::TemperatureMeasurement;
using devices::imtq::Vector3;

namespace
{
    class ExperimentalDetumblingTest : public testing::Test
    {
      protected:
        ExperimentalDetumblingTest();

        testing::NiceMock<ImtqDriverMock> _imtqDriver;
        testing::NiceMock<PowerControlMock> _power;
        testing::NiceMock<OSMock> _os;
        OSReset _osReset;

        adcs::ExperimentalDetumbling _detumbling{_imtqDriver, _power};

        SelfTestResult CreateSuccessfulSelfTestResult() const;
    };

    ExperimentalDetumblingTest::ExperimentalDetumblingTest()
    {
        _osReset = InstallProxy(&_os);
    }

    TEST_F(ExperimentalDetumblingTest, ShouldStartDipoleActuation)
    {
        EXPECT_CALL(_power, ImtqPower(true)).WillOnce(Return(true));
        EXPECT_CALL(_os, GiveSemaphore(_)).WillRepeatedly(Return(OSResult::Success));

        auto expectedDipole = Vector3<Dipole>{-32768, 32767, -32768};

        auto calibratedMagnetometerMeasurement = Vector3<MagnetometerMeasurement>{1, -2, 3};

        auto selfTestResult = CreateSuccessfulSelfTestResult();

        ON_CALL(_imtqDriver, PerformSelfTest(_, _)).WillByDefault(DoAll(SetArgReferee<0>(selfTestResult), Return(true)));

        ON_CALL(_imtqDriver, MeasureMagnetometer(_))
            .WillByDefault(DoAll(SetArgReferee<0>(calibratedMagnetometerMeasurement), Return(true)));
        EXPECT_CALL(_imtqDriver, StartActuationDipole(Eq(expectedDipole), Eq(500ms)));

        _detumbling.Initialize();
        _detumbling.Enable();
        _detumbling.Process();
    }

    TEST_F(ExperimentalDetumblingTest, ShouldOperateWithOneMagnetometerFail)
    {
        EXPECT_CALL(_os, TakeSemaphore(_, _)).WillRepeatedly(Return(OSResult::Success));
        EXPECT_CALL(_os, GiveSemaphore(_)).WillRepeatedly(Return(OSResult::Success));
        EXPECT_CALL(_power, ImtqPower(true)).WillOnce(Return(true));

        auto calibratedMagnetometerMeasurement = Vector3<MagnetometerMeasurement>{1, -2, 3};

        auto selfTestResult = CreateSuccessfulSelfTestResult();

        selfTestResult.stepResults[1].error = Error(1);

        ON_CALL(_imtqDriver, PerformSelfTest(_, _)).WillByDefault(DoAll(SetArgReferee<0>(selfTestResult), Return(true)));
        ON_CALL(_imtqDriver, MeasureMagnetometer(_))
            .WillByDefault(DoAll(SetArgReferee<0>(calibratedMagnetometerMeasurement), Return(true)));

        _detumbling.Initialize();
        ASSERT_THAT(_detumbling.Enable(), Eq(OSResult::Success));
    }

    TEST_F(ExperimentalDetumblingTest, ShouldReportErrorWhenMoreThanOneMagnetometerFails)
    {
        EXPECT_CALL(_power, ImtqPower(true)).WillOnce(Return(true));
        EXPECT_CALL(_power, ImtqPower(false)).WillOnce(Return(true));
        EXPECT_CALL(_os, TakeSemaphore(_, _)).WillRepeatedly(Return(OSResult::Success));
        EXPECT_CALL(_os, GiveSemaphore(_)).WillRepeatedly(Return(OSResult::Success));

        auto selfTestResult = CreateSuccessfulSelfTestResult();

        selfTestResult.stepResults[1].error = Error(1);
        selfTestResult.stepResults[3].error = Error(1);

        ON_CALL(_imtqDriver, PerformSelfTest(_, _)).WillByDefault(DoAll(SetArgReferee<0>(selfTestResult), Return(true)));

        _detumbling.Initialize();
        _detumbling.SetTryFixIsisErrors(true);

        ASSERT_THAT(_detumbling.Enable(), Eq(OSResult::IOError));
    }

    TEST_F(ExperimentalDetumblingTest, ShouldPowerDownImtqOnDisable)
    {
        EXPECT_CALL(_power, ImtqPower(false));

        _detumbling.Disable();
    }

    SelfTestResult ExperimentalDetumblingTest::CreateSuccessfulSelfTestResult() const
    {
        return {//
            SelfTestResult::StepResult{SelfTestResult::Step::Init,
                Error(),
                Vector3<MagnetometerMeasurement>{},
                Vector3<MagnetometerMeasurement>{},
                Vector3<Current>{},
                Vector3<TemperatureMeasurement>{}},
            SelfTestResult::StepResult{SelfTestResult::Step::Xn,
                Error(),
                Vector3<MagnetometerMeasurement>{},
                Vector3<MagnetometerMeasurement>{},
                Vector3<Current>{},
                Vector3<TemperatureMeasurement>{}},
            SelfTestResult::StepResult{SelfTestResult::Step::Xp,
                Error(),
                Vector3<MagnetometerMeasurement>{},
                Vector3<MagnetometerMeasurement>{},
                Vector3<Current>{},
                Vector3<TemperatureMeasurement>{}},
            SelfTestResult::StepResult{SelfTestResult::Step::Yn,
                Error(),
                Vector3<MagnetometerMeasurement>{},
                Vector3<MagnetometerMeasurement>{},
                Vector3<Current>{},
                Vector3<TemperatureMeasurement>{}},
            SelfTestResult::StepResult{SelfTestResult::Step::Yp,
                Error(),
                Vector3<MagnetometerMeasurement>{},
                Vector3<MagnetometerMeasurement>{},
                Vector3<Current>{},
                Vector3<TemperatureMeasurement>{}},
            SelfTestResult::StepResult{SelfTestResult::Step::Zn,
                Error(),
                Vector3<MagnetometerMeasurement>{},
                Vector3<MagnetometerMeasurement>{},
                Vector3<Current>{},
                Vector3<TemperatureMeasurement>{}},
            SelfTestResult::StepResult{SelfTestResult::Step::Zp,
                Error(),
                Vector3<MagnetometerMeasurement>{},
                Vector3<MagnetometerMeasurement>{},
                Vector3<Current>{},
                Vector3<TemperatureMeasurement>{}},
            SelfTestResult::StepResult{SelfTestResult::Step::Fina,
                Error(),
                Vector3<MagnetometerMeasurement>{},
                Vector3<MagnetometerMeasurement>{},
                Vector3<Current>{},
                Vector3<TemperatureMeasurement>{}}};
    }
}
