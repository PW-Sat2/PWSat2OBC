#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "experiment/leop/leop.hpp"
#include "mock/FsMock.hpp"
#include "mock/GyroMock.hpp"
#include "mock/time.hpp"

using testing::NiceMock;
using testing::Return;
using testing::InSequence;
using testing::Eq;
using testing::_;

using namespace experiment::leop;
using experiments::IterationResult;
using experiments::StartResult;
using namespace std::chrono_literals;
namespace
{
    class LEOPExperimentTest : public testing::Test
    {
      public:
        LEOPExperimentTest();

      protected:
        NiceMock<FsMock> _fs;
        NiceMock<CurrentTimeMock> _time;
        GyroscopeMock _gyro;
        LaunchAndEarlyOrbitPhaseExperiment _exp;
    };

    LEOPExperimentTest::LEOPExperimentTest() : _exp(_gyro, _time, _fs)
    {
        ON_CALL(this->_time, GetCurrentTime()).WillByDefault(Return(Some(10ms)));
        std::array<uint8_t, 1> file = {0};
        this->_fs.AddFile(LaunchAndEarlyOrbitPhaseExperiment::FileName, file);
    }

    TEST_F(LEOPExperimentTest, ShouldFailIfStartedAfterExperimentTime)
    {
        EXPECT_CALL(_time, GetCurrentTime())
            .Times(1)
            .WillOnce(Return(Some(static_cast<std::chrono::milliseconds>(LaunchAndEarlyOrbitPhaseExperiment::ExperimentTimeStop))));
        EXPECT_CALL(_fs, Open(_, _, _)).Times(0);
        EXPECT_CALL(_gyro, read()).Times(0);

        auto r = this->_exp.Start();

        ASSERT_THAT(r, Eq(StartResult::Failure));
    }

    TEST_F(LEOPExperimentTest, ShouldRunIfStartedAtMissionStart)
    {
        EXPECT_CALL(_time, GetCurrentTime()).Times(3);
        EXPECT_CALL(_fs, Open(_, _, _)).Times(1);

        auto r = this->_exp.Start();

        ASSERT_THAT(r, Eq(StartResult::Success));
    }

    TEST_F(LEOPExperimentTest, ShouldAquireGyroDataOnExperimentRunning)
    {
        EXPECT_CALL(_time, GetCurrentTime());
        EXPECT_CALL(_gyro, read()).Times(1);

        auto r = this->_exp.Iteration();

        ASSERT_THAT(r, Eq(IterationResult::WaitForNextCycle));
    }

    TEST_F(LEOPExperimentTest, ShouldFinishExperimentIfAfterExperimentTime)
    {
        EXPECT_CALL(_time, GetCurrentTime())
            .WillOnce(Return(Some(static_cast<std::chrono::milliseconds>(LaunchAndEarlyOrbitPhaseExperiment::ExperimentTimeStop))));
        EXPECT_CALL(_gyro, read()).Times(0);

        auto r = this->_exp.Iteration();

        ASSERT_THAT(r, Eq(IterationResult::Finished));
    }
}
