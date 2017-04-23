#include "experiments.h"
#include <algorithm>
#include <chrono>
#include "base/os.h"
#include "logger/logger.h"

using namespace std::chrono_literals;

namespace experiments
{
    static void TaskEntryPoint(ExperimentController* param)
    {
        param->BackgroundTask();
    }

    ExperimentController::ExperimentController()
        : _iterationCounter(0), //
          _task("Mission experiment", this, TaskEntryPoint)
    {
    }

    void ExperimentController::Initialize()
    {
        this->_sync = System::CreateBinarySemaphore(1);
        System::GiveSemaphore(this->_sync);
        this->_event.Initialize();
        this->_queue.Create();
        this->_task.Create();
    }

    bool ExperimentController::RequestExperiment(ExperimentCode experiment)
    {
        auto inProgress = this->_event.IsSet(Event::InProgress);
        if (inProgress)
            return false;

        Lock lock(this->_sync, InfiniteTimeout);

        this->_requestedExperiment = Some(experiment);
        return true;
    }

    void ExperimentController::AbortExperiment()
    {
        LOG(LOG_LEVEL_WARNING, "Requesting experiment abort");

        this->_event.Set(Event::AbortRequest);
    }

    void ExperimentController::BackgroundTask()
    {
        while (1)
        {
            ExperimentCode experimentType;

            LOG(LOG_LEVEL_INFO, "Waiting experiment to run");

            if (OS_RESULT_FAILED(this->_queue.Pop(experimentType, InfiniteTimeout)))
            {
                return;
            }

            LOGF(LOG_LEVEL_INFO, "Received experiment %d request", experimentType);

            auto experiment = std::find_if(this->_experiments.begin(), this->_experiments.end(), [experimentType](IExperiment* e) {
                return e->Type() == experimentType;
            });

            if (experiment == this->_experiments.end())
            {
                LOG(LOG_LEVEL_ERROR, "No handler for requested experiment");
                continue;
            }

            this->_event.Clear(Event::AbortRequest);
            this->_event.Set(Event::InProgress);

            {
                Lock lock(this->_sync, InfiniteTimeout);
                this->_currentExperiment = Some(experimentType);
                this->_lastIterationResult = None<IterationResult>();
                this->_lastStartResult = None<StartResult>();
                this->_iterationCounter = 0;
            }

            RunExperiment(**experiment);

            {
                Lock lock(this->_sync, InfiniteTimeout);
                this->_currentExperiment = None<ExperimentCode>();
            }

            this->_event.Clear(Event::InProgress);
        }
    }

    void ExperimentController::RunExperiment(IExperiment& experiment)
    {
        LOG(LOG_LEVEL_INFO, "Starting experiment");

        auto startResult = experiment.Start();

        {
            Lock lock(this->_sync, InfiniteTimeout);
            this->_lastStartResult = Some(startResult);
            this->_iterationCounter = 0;
        }

        if (startResult != StartResult::Success)
        {
            LOGF(LOG_LEVEL_ERROR, "Experiment start failed: %d", num(startResult));

            return;
        }

        IterationResult iterationResult;
        do
        {
            if (this->_event.IsSet(Event::AbortRequest))
            {
                this->_event.Clear(Event::AbortRequest);

                LOG(LOG_LEVEL_WARNING, "Aborting experiment");

                iterationResult = IterationResult::Failure;
                break;
            }

            this->_event.Clear(Event::MissionLoopIterationStarted);

            iterationResult = experiment.Iteration();

            {
                Lock lock(this->_sync, InfiniteTimeout);
                this->_lastIterationResult = Some(iterationResult);
                this->_iterationCounter++;
            }

            if (iterationResult == IterationResult::Finished)
            {
                break;
            }

            if (iterationResult == IterationResult::Failure)
            {
                break;
            }

            if (iterationResult == IterationResult::WaitForNextCycle)
            {
                this->_event.Set(Event::MissionLoopNotificationReqested);

                auto flags = this->_event.WaitAny(Event::MissionLoopIterationStarted | Event::AbortRequest, false, InfiniteTimeout);

                if (has_flag(flags, Event::MissionLoopIterationStarted))
                {
                    this->_event.Clear(Event::MissionLoopIterationStarted);
                }
            }
        } while (true);

        LOG(LOG_LEVEL_DEBUG, "Stopping experiment");
        experiment.Stop(iterationResult);

        LOG(LOG_LEVEL_INFO, "Experiment stopped");
    }

    void ExperimentController::SetExperiments(gsl::span<IExperiment*> experiments)
    {
        this->_experiments = experiments;
    }

    bool ExperimentController::InProgress()
    {
        return this->_event.IsSet(Event::InProgress);
    }

    bool ExperimentController::IsExperimentRequested()
    {
        Lock lock(this->_sync, InfiniteTimeout);
        return this->_requestedExperiment.HasValue;
    }

    void ExperimentController::StartExperiment()
    {
        Lock lock(this->_sync, InfiniteTimeout);
        if (!this->_requestedExperiment.HasValue)
        {
            LOG(LOG_LEVEL_WARNING, "Starting experiment without requested experiment");
        }
        this->_queue.Overwrite(this->_requestedExperiment.Value);
        this->_requestedExperiment = None<ExperimentCode>();
    }

    void ExperimentController::NotifyLoopIterationStart()
    {
        auto f = this->_event.WaitAll(Event::MissionLoopNotificationReqested, true, 0ms);
        if (has_flag(f, Event::MissionLoopNotificationReqested))
        {
            this->_event.Set(Event::MissionLoopIterationStarted);
        }
    }

    ExperimentState ExperimentController::CurrentState()
    {
        Lock lock(this->_sync, InfiniteTimeout);

        ExperimentState state;
        state.RequestedExperiment = this->_requestedExperiment;
        state.CurrentExperiment = this->_currentExperiment;
        state.LastStartResult = this->_lastStartResult;
        state.LastStartResult = this->_lastStartResult;
        state.LastIterationResult = this->_lastIterationResult;
        state.IterationCounter = this->_iterationCounter;

        return state;
    }
}
