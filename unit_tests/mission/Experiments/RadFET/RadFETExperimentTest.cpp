#include <chrono>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "OsMock.hpp"
#include "base/writer.h"
#include "experiment/radfet/radfet.hpp"
#include "experiments/experiments.h"
#include "fs/ExperimentFile.hpp"
#include "mock/FsMock.hpp"
#include "mock/PayloadDeviceMock.hpp"
#include "mock/power.hpp"
#include "mock/time.hpp"
#include "power/power.h"

using namespace std::chrono_literals;

using namespace experiment::radfet;
using namespace experiments;
using namespace devices::payload;

using testing::_;
using testing::Eq;
using testing::StrEq;
using testing::Return;
using testing::Invoke;

namespace
{
    class RadFETExperimentTest : public testing::Test
    {
      protected:
        RadFETExperimentTest();

        testing::NiceMock<FsMock> fs;
        testing::NiceMock<PayloadDeviceMock> pld;
        testing::NiceMock<PowerControlMock> power;
        testing::NiceMock<CurrentTimeMock> timeProvider;

        testing::NiceMock<OSMock> os;
        OSReset osReset{InstallProxy(&os)};

        std::chrono::milliseconds time{0ms};
        std::array<std::uint8_t, 900> fileBuf;

        RadFETExperiment exp{fs, pld, power, timeProvider};
    };

    RadFETExperimentTest::RadFETExperimentTest()
    {
        ON_CALL(this->power, SensPower(_)).WillByDefault(Return(true));

        ON_CALL(this->timeProvider, GetCurrentTime()).WillByDefault(Invoke([this]() { return Some(this->time); }));

        this->fileBuf.fill(0xFF);

        this->fs.AddFile("/radfet", this->fileBuf);
    }

    TEST_F(RadFETExperimentTest, TestStart)
    {
        EXPECT_CALL(this->fs, Open(StrEq("/radfet"), _, _)).WillOnce(Return(MakeOpenedFile(1)));

        EXPECT_CALL(this->power, SensPower(true)).WillOnce(Return(true));
        EXPECT_CALL(this->pld, RadFETOn(_)).WillOnce(Return(OSResult::Success));
        EXPECT_CALL(this->os, Sleep(Eq(3s))).Times(1);
        EXPECT_CALL(this->os, Sleep(Eq(20s))).Times(1);

        this->exp.Setup(std::chrono::seconds(20), 5, "/radfet");

        ASSERT_THAT(this->exp.Start(), Eq(StartResult::Success));
    }

    TEST_F(RadFETExperimentTest, TestLoop)
    {
        EXPECT_CALL(this->pld, MeasureRadFET(_)).WillOnce(Return(OSResult::Success));
        EXPECT_CALL(this->pld, MeasureTemperatures(_)).WillOnce(Return(OSResult::Success));

        ASSERT_THAT(this->exp.Iteration(), Eq(IterationResult::Finished));
    }

    TEST_F(RadFETExperimentTest, TestEnd)
    {
        EXPECT_CALL(this->pld, RadFETOff(_)).WillOnce(Return(OSResult::Success));
        EXPECT_CALL(this->power, SensPower(false)).WillOnce(Return(true));

        this->exp.Stop(IterationResult::Finished);
    }

    TEST_F(RadFETExperimentTest, TestWholeCycle)
    {
        ON_CALL(this->fs, Open(StrEq("/data_file"), _, _)).WillByDefault(Return(MakeOpenedFile(1)));

        ON_CALL(this->power, SensPower(_)).WillByDefault(Return(true));

        ON_CALL(this->pld, RadFETOn(_)).WillByDefault(Return(OSResult::Success));
        ON_CALL(this->pld, RadFETOff(_)).WillByDefault(Return(OSResult::Success));

        ON_CALL(this->pld, MeasureRadFET(_)).WillByDefault(Return(OSResult::Success));
        ON_CALL(this->pld, MeasureTemperatures(_)).WillByDefault(Return(OSResult::Success));

        this->exp.Setup(std::chrono::seconds(20), 5, "/data_file");

        this->exp.Start();

        ASSERT_THAT(this->exp.Iteration(), Eq(IterationResult::WaitForNextCycle)) << "Iteration 1";
        ASSERT_THAT(this->exp.Iteration(), Eq(IterationResult::WaitForNextCycle)) << "Iteration 2";
        ASSERT_THAT(this->exp.Iteration(), Eq(IterationResult::WaitForNextCycle)) << "Iteration 3";
        ASSERT_THAT(this->exp.Iteration(), Eq(IterationResult::WaitForNextCycle)) << "Iteration 4";
        ASSERT_THAT(this->exp.Iteration(), Eq(IterationResult::Finished)) << "Iteration 5";

        this->exp.Stop(IterationResult::Finished);
    }
}
