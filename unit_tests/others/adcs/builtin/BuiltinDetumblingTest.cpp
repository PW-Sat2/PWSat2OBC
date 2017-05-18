#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "adcs/BuiltinDetumbling.hpp"
#include "mock/ImtqDriverMock.hpp"

using testing::_;

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
}
