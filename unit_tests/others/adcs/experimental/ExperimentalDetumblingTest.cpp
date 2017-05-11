#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "adcs/ExperimentalDetumbling.hpp"
#include "mock/ImtqDriverMock.hpp"

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
        testing::NiceMock<ImtqDriverMock> _imtqDriver;

        adcs::ExperimentalDetumbling _detumbling{_imtqDriver};

        SelfTestResult CreateSuccessfulSelfTestResult() const;
    };

    TEST_F(ExperimentalDetumblingTest, ShouldStartDipoleActuation)
    {
        auto detumbleData = DetumbleData{};
        auto expectedDipole = Vector3<Dipole>{-22464, 20608, -1920};

        detumbleData.calibratedMagnetometerMeasurement = Vector3<MagnetometerMeasurement>{1, 2, 3};

        auto selfTestResult = CreateSuccessfulSelfTestResult();

        ON_CALL(_imtqDriver, StartAllAxisSelfTest()).WillByDefault(Return(true));
        ON_CALL(_imtqDriver, GetSelfTestResult(_)).WillByDefault(DoAll(SetArgReferee<0>(selfTestResult), Return(true)));

        ON_CALL(_imtqDriver, GetDetumbleData(_)).WillByDefault(DoAll(SetArgReferee<0>(detumbleData), Return(true)));
        EXPECT_CALL(_imtqDriver, StartActuationDipole(Eq(expectedDipole), Eq(0ms)));

        _detumbling.Enable();
        _detumbling.Process();
    }

    TEST_F(ExperimentalDetumblingTest, ShouldOperateWithOneMagnetometerFail)
    {
        auto selfTestResult = CreateSuccessfulSelfTestResult();

        selfTestResult.stepResults[1].error = Error(1);

        ON_CALL(_imtqDriver, StartAllAxisSelfTest()).WillByDefault(Return(true));
        ON_CALL(_imtqDriver, GetSelfTestResult(_)).WillByDefault(DoAll(SetArgReferee<0>(selfTestResult), Return(true)));

        ASSERT_THAT(_detumbling.Enable(), Eq(OSResult::Success));
    }

    TEST_F(ExperimentalDetumblingTest, ShouldReportErrorWhenMoreThanOneMagnetometerFails)
    {
        auto selfTestResult = CreateSuccessfulSelfTestResult();

        selfTestResult.stepResults[1].error = Error(1);
        selfTestResult.stepResults[3].error = Error(1);

        ON_CALL(_imtqDriver, StartAllAxisSelfTest()).WillByDefault(Return(true));
        ON_CALL(_imtqDriver, GetSelfTestResult(_)).WillByDefault(DoAll(SetArgReferee<0>(selfTestResult), Return(true)));

        ASSERT_THAT(_detumbling.Enable(), Eq(OSResult::IOError));
    }

    SelfTestResult ExperimentalDetumblingTest::CreateSuccessfulSelfTestResult() const
    {
        return {SelfTestResult::StepResult{SelfTestResult::Step::Init,
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
