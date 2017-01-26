#include "experiments.h"
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

        MissionExperiment::MissionExperiment(void*)
            : _requestedExperiment(None<Experiment>()), _task("Mission experiment", this, TaskEntryPoint)
        {
        }

        void MissionExperiment::Initialize()
        {
            this->_event = System::CreateEventGroup();
            this->_queue.Create();
            this->_task.Create();
        }

        void MissionExperiment::RequestExperiment(Experiment experiment)
        {
            auto inProgress = (System::EventGroupGetBits(this->_event) & Event::InProgress) == Event::InProgress;
            if (inProgress)
                return;

            this->_requestedExperiment = Some(experiment);
        }

        MissionExperimentComponent::MissionExperimentComponent(MissionExperiment& experimentController)
            : _experimentController(experimentController)
        {
        }

        mission::ActionDescriptor<SystemState> MissionExperimentComponent::BuildAction()
        {
            auto d = mission::ActionDescriptor<SystemState>();

            d.name = "StartExp";
            d.param = &this->_experimentController;
            d.condition = MissionExperiment::ShouldStartExperiment;
            d.actionProc = MissionExperiment::StartExperiment;

            return d;
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

        void MissionExperiment::BackgroundTask()
        {
            while (1)
            {
                Experiment experimentType;

                LOG(LOG_LEVEL_INFO, "Waiting experiment to run");

                System::EventGroupClearBits(this->_event, 1 << 2);

                if (OS_RESULT_FAILED(this->_queue.Pop(experimentType, InfiniteTimeout)))
                {
                    return;
                }

                LOGF(LOG_LEVEL_INFO, "Received experiment %d request", num(experimentType));

                IExperiment* experiment = nullptr;

                for (auto e : this->_experiments)
                {
                    if (e->Type() == experimentType)
                    {
                        experiment = e;
                        break;
                    }
                }

                if (experiment == nullptr)
                {
                    LOG(LOG_LEVEL_ERROR, "No handler for requested experiment");
                    continue;
                }

                System::EventGroupSetBits(this->_event, Event::InProgress);

                ExperimentContext context(this->_event);

                experiment->Run(context);
            }
        }

        void MissionExperiment::SetExperiments(gsl::span<IExperiment*> experiments)
        {
            this->_experiments = experiments;
        }

        void MissionExperiment::StartExperiment(const SystemState& state, void* param)
        {
            UNREFERENCED_PARAMETER(state);

            auto This = reinterpret_cast<MissionExperiment*>(param);

            This->_queue.Overwrite(This->_requestedExperiment.Value);
            This->_requestedExperiment = None<Experiment>();

            System::EventGroupSetBits(This->_event, 1 << 2);
        }

        ExperimentContext::ExperimentContext(OSEventGroupHandle eventGroup) : _eventGroup(eventGroup)
        {
        }

        void ExperimentContext::WaitForNextCycle()
        {
            System::EventGroupWaitForBits(this->_eventGroup, 1 << 2, true, true, InfiniteTimeout);
        }
    }
}
