#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "adcs/ExperimentalDetumbling.hpp"
#include "mock/ImtqDriverMock.hpp"

using testing::Eq;
using testing::Gt;
using testing::_;
using testing::Return;
using testing::DoAll;
using testing::SetArgReferee;

namespace
{
    class ExperimentalDetumblingTest : public testing::Test
    {
      protected:
        testing::NiceMock<ImtqDriverMock> _imtqDriver;

        adcs::ExperimentalDetumbling _detumbling{_imtqDriver};
    };

    TEST_F(ExperimentalDetumblingTest, ShouldStartDipoleActuation)
    {
        auto minActuationTime = std::chrono::period_cast<std::chrono::milliseconds>(adcs::ExperimentalDetumbling::Frequency);
        auto measurement = devices::imtq::MagnetometerMeasurementResult{};
        auto expectedDipole = devices::imtq::Vector3<devices::imtq::Dipole>{0, 0, 0};

        ON_CALL(_imtqDriver, GetCalibratedMagnetometerData(_)).WillByDefault(DoAll(SetArgReferee<0>(measurement), Return(true)));
        EXPECT_CALL(_imtqDriver, StartActuationDipole(Eq(expectedDipole), Gt(minActuationTime)));

        _detumbling.Enable();
        _detumbling.Process();
    }
}
