#include "experiments.h"
#include <algorithm>
#include "base/os.h"
#include "logger/logger.h"

namespace mission
{
    namespace experiments
    {
        static void TaskEntryPoint(MissionExperiment* param)
        {
            param->BackgroundTask();
        }

        MissionExperiment::MissionExperiment()
            : _requestedExperiment(None<ExperimentCode>()), _task("Mission experiment", this, TaskEntryPoint)
        {
        }

        void MissionExperiment::Initialize()
        {
            this->_event = System::CreateEventGroup();
            this->_queue.Create();
            this->_task.Create();
        }

        void MissionExperiment::RequestExperiment(ExperimentCode experiment)
        {
            auto inProgress = (System::EventGroupGetBits(this->_event) & Event::InProgress) == Event::InProgress;
            if (inProgress)
                return;

            this->_requestedExperiment = Some(experiment);
        }

        void MissionExperiment::AbortExperiment()
        {
            LOG(LOG_LEVEL_WARNING, "Requesting experiment abort");

            System::EventGroupSetBits(this->_event, Event::AbortRequest);
        }

        void MissionExperiment::BackgroundTask()
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

                System::EventGroupSetBits(this->_event, Event::InProgress);

                RunExperiment(**experiment);

                System::EventGroupClearBits(this->_event, Event::InProgress);
            }
        }

        void MissionExperiment::RunExperiment(IExperiment& experiment)
        {
            LOG(LOG_LEVEL_INFO, "Starting experiment");

            auto startResult = experiment.Start();

            if (startResult != StartResult::Success)
            {
                LOGF(LOG_LEVEL_ERROR, "Experiment start failed: %d", num(startResult));

                return;
            }

            IterationResult iterationResult;
            do
            {
                auto flags = System::EventGroupGetBits(this->_event);

                if (has_flag(flags, Event::AbortRequest))
                {
                    System::EventGroupClearBits(this->_event, Event::AbortRequest);

                    LOG(LOG_LEVEL_WARNING, "Aborting experiment");

                    iterationResult = IterationResult::Failure;
                    break;
                }

                System::EventGroupClearBits(this->_event, Event::MissionLoopIterationStarted);

                iterationResult = experiment.Iteration();

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
                    auto flags = System::EventGroupWaitForBits(
                        this->_event, Event::MissionLoopIterationStarted | Event::AbortRequest, false, false, InfiniteTimeout);

                    if (has_flag(flags, Event::MissionLoopIterationStarted))
                    {
                        System::EventGroupClearBits(this->_event, Event::MissionLoopIterationStarted);
                    }
                }
            } while (true);

            LOG(LOG_LEVEL_DEBUG, "Stopping experiment");
            experiment.Stop(iterationResult);

            LOG(LOG_LEVEL_INFO, "Experiment stopped");
        }

        void MissionExperiment::SetExperiments(gsl::span<IExperiment*> experiments)
        {
            this->_experiments = experiments;
        }

        bool MissionExperiment::ShouldStartExperiment(const SystemState& state, void* param)
        {
            UNREFERENCED_PARAMETER(state);

            auto This = reinterpret_cast<MissionExperiment*>(param);

            auto inProgress = has_flag(System::EventGroupGetBits(This->_event), Event::InProgress);

            if (inProgress)
                return false;

            return This->_requestedExperiment.HasValue;
        }

        void MissionExperiment::StartExperiment(const SystemState& state, void* param)
        {
            UNREFERENCED_PARAMETER(state);

            auto This = reinterpret_cast<MissionExperiment*>(param);

            This->_queue.Overwrite(This->_requestedExperiment.Value);
            This->_requestedExperiment = None<ExperimentCode>();
        }

        bool MissionExperiment::ShouldKickExperiment(const SystemState& state, void* param)
        {
            UNREFERENCED_PARAMETER(state);

            auto This = reinterpret_cast<MissionExperiment*>(param);

            auto inProgress = has_flag(System::EventGroupGetBits(This->_event), Event::InProgress);

            return inProgress;
        }

        void MissionExperiment::KickExperiment(const SystemState& state, void* param)
        {
            UNREFERENCED_PARAMETER(state);

            auto This = reinterpret_cast<MissionExperiment*>(param);

            System::EventGroupSetBits(This->_event, 1 << 2);
        }

        mission::ActionDescriptor<SystemState> MissionExperiment::StartExperimentAction()
        {
            auto d = mission::ActionDescriptor<SystemState>();

            d.name = "StartExp";
            d.param = this;
            d.condition = MissionExperiment::ShouldStartExperiment;
            d.actionProc = MissionExperiment::StartExperiment;

            return d;
        }

        mission::ActionDescriptor<SystemState> MissionExperiment::KickExperimentAction()
        {
            auto d = mission::ActionDescriptor<SystemState>();

            d.name = "KickExp";
            d.param = this;
            d.condition = MissionExperiment::ShouldKickExperiment;
            d.actionProc = MissionExperiment::KickExperiment;

            return d;
        }

        MissionExperimentComponent::MissionExperimentComponent(MissionExperiment& experimentController)
            : _experimentController(experimentController), //
              _actions("Experiment", experimentController.StartExperimentAction(), experimentController.KickExperimentAction())
        {
        }

        mission::ActionDescriptor<SystemState> MissionExperimentComponent::BuildAction()
        {
            return this->_actions.BuildAction();
        }
    }
}
