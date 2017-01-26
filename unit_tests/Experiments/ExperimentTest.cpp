#include <chrono>
#include <cstdint>

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "OsMock.hpp"
#include "base/os.h"
#include "mission/base.hpp"
#include "mission/experiments.h"
#include "system.h"
#include "utils.h"
#include "utils.hpp"

using testing::Eq;
using testing::NiceMock;
using testing::Return;
using testing::Pointee;
using testing::Invoke;
using testing::_;
using testing::MatcherCast;
using testing::InSequence;
using testing::SaveArg;
using testing::ReturnPointee;
using testing::DoAll;
using testing::Assign;
using namespace mission::experiments;

struct ExperimentMock : public IExperiment
{
    MOCK_METHOD0(Type, Experiment());
    MOCK_METHOD1(Run, void(ExperimentContext& context));
};

class ExperimentTest : public testing::Test
{
  public:
    ExperimentTest();

  protected:
    const OSEventGroupHandle _event = reinterpret_cast<OSEventGroupHandle>(2);
    const OSQueueHandle _queue = reinterpret_cast<OSQueueHandle>(3);

    OSEventBits _eventValue;

    MissionExperiment _exp;
    MissionExperimentComponent _mission;
    NiceMock<OSMock> _os;
    OSReset _osReset;
};

ExperimentTest::ExperimentTest() : _exp(nullptr), _mission(_exp)
{
    this->_osReset = InstallProxy(&this->_os);
    this->_eventValue = 0;

    ON_CALL(this->_os, CreateEventGroup()).WillByDefault(Return(this->_event));
    ON_CALL(this->_os, CreateQueue(1, sizeof(Experiment))).WillByDefault(Return(this->_queue));

    ON_CALL(this->_os, EventGroupSetBits(this->_event, _))
        .WillByDefault(DoAll(SaveArg<1>(&this->_eventValue), ReturnPointee(&this->_eventValue)));

    ON_CALL(this->_os, EventGroupGetBits(this->_event)).WillByDefault(ReturnPointee(&this->_eventValue));

    ON_CALL(this->_os, QueueOverwrite(this->_queue, _)).WillByDefault(Assign(&this->_eventValue, MissionExperiment::Event::InProgress));

    this->_exp.Initialize();
}

MATCHER_P(Exp, e, "")
{
    const auto v = reinterpret_cast<const Experiment*>(arg);
    return *v == e;
}

TEST_F(ExperimentTest, ShouldStartRequestedExperiment)
{
    EXPECT_CALL(this->_os, QueueOverwrite(this->_queue, Exp(Experiment::Fibo)));
    SystemState state;

    auto action = _mission.BuildAction();

    _exp.RequestExperiment(Experiment::Fibo);

    ASSERT_THAT(action.EvaluateCondition(state), Eq(true));
    action.Execute(state);
}

TEST_F(ExperimentTest, RepeatedExperimentRequestOverwritePrevious)
{
    EXPECT_CALL(this->_os, QueueOverwrite(this->_queue, Exp(Experiment::Experiment2)));

    SystemState state;

    auto action = _mission.BuildAction();

    _exp.RequestExperiment(Experiment::Fibo);
    _exp.RequestExperiment(Experiment::Experiment2);

    ASSERT_THAT(action.EvaluateCondition(state), Eq(true));
    action.Execute(state);
}

TEST_F(ExperimentTest, ShouldIgnoreRequestWhileExperimentIsRunning)
{
    SystemState state;
    auto action = _mission.BuildAction();

    _exp.RequestExperiment(Experiment::Fibo);

    action.Execute(state);

    _exp.RequestExperiment(Experiment::Experiment2);

    this->_eventValue = 0;

    ASSERT_THAT(action.EvaluateCondition(state), Eq(false));
}

TEST_F(ExperimentTest, OnceExperimentIsStartedWillNotTryToStartAgain)
{
    SystemState state;
    auto action = _mission.BuildAction();

    _exp.RequestExperiment(Experiment::Fibo);

    action.Execute(state);

    ASSERT_THAT(action.EvaluateCondition(state), Eq(false));
}

TEST_F(ExperimentTest, ShouldInvokeExperimentAsRequested)
{
    NiceMock<ExperimentMock> experiment;
    ON_CALL(experiment, Type()).WillByDefault(Return(Experiment::Fibo));
    EXPECT_CALL(experiment, Run(_));

    {
        InSequence s;

        EXPECT_CALL(this->_os, QueueReceive(this->_queue, _, _))
            .WillOnce(Invoke([](OSQueueHandle /*queue*/, void* element, std::chrono::milliseconds /** timeout*/) {
                *reinterpret_cast<Experiment*>(element) = Experiment::Fibo;
                return true;
            }));

        EXPECT_CALL(this->_os, QueueReceive(this->_queue, _, _)).WillOnce(Return(false));
    }

    IExperiment* experiments[] = {&experiment};
    _exp.SetExperiments(experiments);

    _exp.BackgroundTask();
}

TEST_F(ExperimentTest, ShouldInvokeOneExperimentAfterAnother)
{
    NiceMock<ExperimentMock> experiment1;
    ON_CALL(experiment1, Type()).WillByDefault(Return(Experiment::Fibo));
    EXPECT_CALL(experiment1, Run(_));

    NiceMock<ExperimentMock> experiment2;
    ON_CALL(experiment2, Type()).WillByDefault(Return(Experiment::Experiment2));
    EXPECT_CALL(experiment2, Run(_));

    {
        InSequence s;

        EXPECT_CALL(this->_os, QueueReceive(this->_queue, _, _))
            .WillOnce(Invoke([](OSQueueHandle /*queue*/, void* element, std::chrono::milliseconds /** timeout*/) {
                *reinterpret_cast<Experiment*>(element) = Experiment::Experiment2;
                return true;
            }));

        EXPECT_CALL(this->_os, QueueReceive(this->_queue, _, _))
            .WillOnce(Invoke([](OSQueueHandle /*queue*/, void* element, std::chrono::milliseconds /** timeout*/) {
                *reinterpret_cast<Experiment*>(element) = Experiment::Fibo;
                return true;
            }));

        EXPECT_CALL(this->_os, QueueReceive(this->_queue, _, _)).WillOnce(Return(false));
    }

    IExperiment* experiments[] = {&experiment1, &experiment2};
    _exp.SetExperiments(experiments);

    _exp.BackgroundTask();
}
