#include <chrono>
#include <cstdint>

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "OsMock.hpp"
#include "base/os.h"
#include "experiments/experiments.h"
#include "mission/base.hpp"
#include "mission/experiments.hpp"
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
using namespace experiments;
using namespace mission::experiments;
using namespace std::chrono_literals;
namespace
{
    struct ExperimentMock : public IExperiment
    {
        MOCK_METHOD0(Type, ExperimentCode());
        MOCK_METHOD0(Start, StartResult());
        MOCK_METHOD0(Iteration, IterationResult());
        MOCK_METHOD1(Stop, void(IterationResult lastResult));
    };

    struct Experiment
    {
        static constexpr ExperimentCode Fibo = 0x01;
        static constexpr ExperimentCode Experiment2 = 0x02;
    };

    class ExperimentTest : public testing::Test
    {
      public:
        ExperimentTest();

      protected:
        const OSEventGroupHandle _event = reinterpret_cast<OSEventGroupHandle>(2);
        const OSQueueHandle _queue = reinterpret_cast<OSQueueHandle>(3);

        OSEventBits _eventValue;

        ExperimentController _exp;
        MissionExperimentComponent _mission;
        NiceMock<OSMock> _os;
        OSReset _osReset;
    };

    ExperimentTest::ExperimentTest() : _mission(_exp)
    {
        this->_osReset = InstallProxy(&this->_os);
        this->_eventValue = 0;

        ON_CALL(this->_os, CreateEventGroup()).WillByDefault(Return(this->_event));
        ON_CALL(this->_os, CreateQueue(1, sizeof(ExperimentCode))).WillByDefault(Return(this->_queue));

        ON_CALL(this->_os, EventGroupSetBits(this->_event, _))
            .WillByDefault(DoAll(SaveArg<1>(&this->_eventValue), ReturnPointee(&this->_eventValue)));

        ON_CALL(this->_os, EventGroupGetBits(this->_event)).WillByDefault(ReturnPointee(&this->_eventValue));

        ON_CALL(this->_os, QueueOverwrite(this->_queue, _))
            .WillByDefault(Assign(&this->_eventValue, ExperimentController::Event::InProgress));

        this->_exp.Initialize();
    }

    MATCHER_P(Exp, e, "")
    {
        const auto v = reinterpret_cast<const ExperimentCode*>(arg);
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
        auto startAction = _mission.StartExperimentAction();

        auto r = _exp.RequestExperiment(Experiment::Fibo);
        ASSERT_THAT(r, Eq(true));

        startAction.Execute(state);
        this->_eventValue = ExperimentController::Event::InProgress;

        r = _exp.RequestExperiment(Experiment::Experiment2);
        ASSERT_THAT(r, Eq(false));

        this->_eventValue = 0;

        ASSERT_THAT(startAction.EvaluateCondition(state), Eq(false));
    }

    TEST_F(ExperimentTest, OnceExperimentIsStartedWillNotTryToStartAgain)
    {
        SystemState state;
        auto startAction = _mission.StartExperimentAction();

        _exp.RequestExperiment(Experiment::Fibo);

        startAction.Execute(state);

        ASSERT_THAT(startAction.EvaluateCondition(state), Eq(false));
    }

    TEST_F(ExperimentTest, ShouldInvokeExperimentAsRequested)
    {
        NiceMock<ExperimentMock> experiment;
        ON_CALL(experiment, Type()).WillByDefault(Return(Experiment::Fibo));
        EXPECT_CALL(experiment, Start());
        EXPECT_CALL(experiment, Iteration()).WillOnce(Return(IterationResult::Finished));
        EXPECT_CALL(experiment, Stop(IterationResult::Finished));

        {
            InSequence s;

            EXPECT_CALL(this->_os, QueueReceive(this->_queue, _, _))
                .WillOnce(Invoke([](OSQueueHandle /*queue*/, void* element, std::chrono::milliseconds /** timeout*/) {
                    *reinterpret_cast<ExperimentCode*>(element) = Experiment::Fibo;
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
        EXPECT_CALL(experiment1, Start());
        EXPECT_CALL(experiment1, Iteration()).WillOnce(Return(IterationResult::Finished));
        EXPECT_CALL(experiment1, Stop(IterationResult::Finished));

        NiceMock<ExperimentMock> experiment2;
        ON_CALL(experiment2, Type()).WillByDefault(Return(Experiment::Experiment2));
        EXPECT_CALL(experiment2, Start());
        EXPECT_CALL(experiment2, Iteration()).WillOnce(Return(IterationResult::Finished));
        EXPECT_CALL(experiment2, Stop(IterationResult::Finished));

        {
            InSequence s;

            EXPECT_CALL(this->_os, QueueReceive(this->_queue, _, _))
                .WillOnce(Invoke([](OSQueueHandle /*queue*/, void* element, std::chrono::milliseconds /** timeout*/) {
                    *reinterpret_cast<ExperimentCode*>(element) = Experiment::Experiment2;
                    return true;
                }));

            EXPECT_CALL(this->_os, QueueReceive(this->_queue, _, _))
                .WillOnce(Invoke([](OSQueueHandle /*queue*/, void* element, std::chrono::milliseconds /** timeout*/) {
                    *reinterpret_cast<ExperimentCode*>(element) = Experiment::Fibo;
                    return true;
                }));

            EXPECT_CALL(this->_os, QueueReceive(this->_queue, _, _)).WillOnce(Return(false));
        }

        IExperiment* experiments[] = {&experiment1, &experiment2};
        _exp.SetExperiments(experiments);

        _exp.BackgroundTask();
    }

    TEST_F(ExperimentTest, ShouldAbortExperiment)
    {
        NiceMock<ExperimentMock> experiment;
        ON_CALL(experiment, Type()).WillByDefault(Return(Experiment::Fibo));
        ON_CALL(experiment, Iteration()).WillByDefault(Return(IterationResult::LoopImmediately));

        {
            InSequence s;

            EXPECT_CALL(this->_os, QueueReceive(this->_queue, _, _))
                .WillOnce(Invoke([](OSQueueHandle /*queue*/, void* element, std::chrono::milliseconds /** timeout*/) {
                    *reinterpret_cast<ExperimentCode*>(element) = Experiment::Fibo;
                    return true;
                }));

            EXPECT_CALL(this->_os, EventGroupClearBits(this->_event, ExperimentController::Event::AbortRequest));
            EXPECT_CALL(this->_os, EventGroupSetBits(this->_event, ExperimentController::Event::InProgress));

            EXPECT_CALL(experiment, Start());

            EXPECT_CALL(this->_os, EventGroupWaitForBits(this->_event, ExperimentController::Event::AbortRequest, false, true, 0ms))
                .WillOnce(Return(0));

            EXPECT_CALL(this->_os, EventGroupClearBits(this->_event, ExperimentController::Event::MissionLoopIterationStarted));

            EXPECT_CALL(this->_os, EventGroupWaitForBits(this->_event, ExperimentController::Event::AbortRequest, false, true, 0ms))
                .WillOnce(Return(ExperimentController::Event::AbortRequest));

            EXPECT_CALL(experiment, Stop(IterationResult::Failure));

            EXPECT_CALL(this->_os, EventGroupClearBits(this->_event, ExperimentController::Event::InProgress));

            EXPECT_CALL(this->_os, QueueReceive(this->_queue, _, _)).WillOnce(Return(false));
        }

        IExperiment* experiments[] = {&experiment};
        _exp.SetExperiments(experiments);

        _exp.BackgroundTask();
    }

    TEST_F(ExperimentTest, ShouldWaitForNextMissionLoopIterationIfRequested)
    {
        NiceMock<ExperimentMock> experiment;
        ON_CALL(experiment, Type()).WillByDefault(Return(Experiment::Fibo));

        {
            InSequence s;

            EXPECT_CALL(this->_os, QueueReceive(this->_queue, _, _))
                .WillOnce(Invoke([](OSQueueHandle /*queue*/, void* element, std::chrono::milliseconds /** timeout*/) {
                    *reinterpret_cast<ExperimentCode*>(element) = Experiment::Fibo;
                    return true;
                }));

            EXPECT_CALL(this->_os, EventGroupClearBits(this->_event, ExperimentController::Event::AbortRequest));
            EXPECT_CALL(this->_os, EventGroupSetBits(this->_event, ExperimentController::Event::InProgress));

            EXPECT_CALL(experiment, Start());

            EXPECT_CALL(this->_os, EventGroupWaitForBits(this->_event, ExperimentController::Event::AbortRequest, false, true, 0ms))
                .WillOnce(Return(false));

            EXPECT_CALL(this->_os, EventGroupClearBits(this->_event, ExperimentController::Event::MissionLoopIterationStarted));

            EXPECT_CALL(experiment, Iteration()).WillOnce(Return(IterationResult::WaitForNextCycle));

            EXPECT_CALL(this->_os, EventGroupSetBits(this->_event, ExperimentController::Event::MissionLoopNotificationReqested));

            EXPECT_CALL(this->_os,
                EventGroupWaitForBits(this->_event,
                    ExperimentController::Event::MissionLoopIterationStarted | ExperimentController::Event::AbortRequest,
                    false,
                    false,
                    InfiniteTimeout))
                .WillOnce(Return(ExperimentController::Event::MissionLoopIterationStarted));

            EXPECT_CALL(this->_os, EventGroupWaitForBits(this->_event, ExperimentController::Event::AbortRequest, false, true, 0ms))
                .WillOnce(Return(ExperimentController::Event::MissionLoopIterationStarted));

            EXPECT_CALL(this->_os, EventGroupClearBits(this->_event, ExperimentController::Event::MissionLoopIterationStarted));

            EXPECT_CALL(experiment, Iteration()).WillOnce(Return(IterationResult::Finished));

            EXPECT_CALL(this->_os, EventGroupClearBits(this->_event, ExperimentController::Event::InProgress));

            EXPECT_CALL(this->_os, QueueReceive(this->_queue, _, _)).WillOnce(Return(false));
        }

        IExperiment* experiments[] = {&experiment};
        _exp.SetExperiments(experiments);

        _exp.BackgroundTask();
    }

    TEST_F(ExperimentTest, WhenExperimentStartFailsShouldNotRunIteration)
    {
        NiceMock<ExperimentMock> experiment;
        ON_CALL(experiment, Type()).WillByDefault(Return(Experiment::Fibo));

        {
            InSequence s;

            EXPECT_CALL(this->_os, QueueReceive(this->_queue, _, _))
                .WillOnce(Invoke([](OSQueueHandle /*queue*/, void* element, std::chrono::milliseconds /** timeout*/) {
                    *reinterpret_cast<ExperimentCode*>(element) = Experiment::Fibo;
                    return true;
                }));

            EXPECT_CALL(experiment, Start()).WillOnce(Return(StartResult::Failure));

            EXPECT_CALL(experiment, Iteration()).Times(0);
            EXPECT_CALL(experiment, Stop(_)).Times(0);

            EXPECT_CALL(this->_os, QueueReceive(this->_queue, _, _)).WillOnce(Return(false));
        }

        IExperiment* experiments[] = {&experiment};
        _exp.SetExperiments(experiments);

        _exp.BackgroundTask();
    }

    TEST_F(ExperimentTest, ShouldStopExperimentWhenIterationFails)
    {
        NiceMock<ExperimentMock> experiment;
        ON_CALL(experiment, Type()).WillByDefault(Return(Experiment::Fibo));

        {
            InSequence s;

            EXPECT_CALL(this->_os, QueueReceive(this->_queue, _, _))
                .WillOnce(Invoke([](OSQueueHandle /*queue*/, void* element, std::chrono::milliseconds /** timeout*/) {
                    *reinterpret_cast<ExperimentCode*>(element) = Experiment::Fibo;
                    return true;
                }));

            EXPECT_CALL(experiment, Start());

            EXPECT_CALL(experiment, Iteration()).Times(2).WillRepeatedly(Return(IterationResult::LoopImmediately));
            EXPECT_CALL(experiment, Iteration()).WillOnce(Return(IterationResult::Failure));
            EXPECT_CALL(experiment, Stop(IterationResult::Failure));

            EXPECT_CALL(this->_os, QueueReceive(this->_queue, _, _)).WillOnce(Return(false));
        }

        IExperiment* experiments[] = {&experiment};
        _exp.SetExperiments(experiments);

        _exp.BackgroundTask();
    }

    TEST_F(ExperimentTest, ShouldUpdateSystemStateWhenNoExperimentIsRunning)
    {
        SystemState state;

        NiceMock<ExperimentMock> experiment;
        ON_CALL(experiment, Type()).WillByDefault(Return(Experiment::Fibo));

        _mission.BuildUpdate().Execute(state);

        ASSERT_THAT(state.Experiment.CurrentExperiment, Eq(None<ExperimentCode>()));
    }
}
