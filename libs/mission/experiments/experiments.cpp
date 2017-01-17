#include "experiments.h"

namespace mission
{
    namespace experiments
    {
        MissionExperiment::MissionExperiment() : _requestedExperiment(None<Experiment>())
        {
        }

        void MissionExperiment::Initialize()
        {
            this->_event = System::CreateEventGroup();
            this->_queue.Create();
        }

        void MissionExperiment::RequestExperiment(Experiment experiment)
        {
            auto inProgress = (System::EventGroupGetBits(this->_event) & Event::InProgress) == Event::InProgress;
            if (inProgress)
                return;

            this->_requestedExperiment = Some(experiment);
        }

        mission::ActionDescriptor<SystemState> MissionExperiment::BuildAction()
        {
            auto d = mission::ActionDescriptor<SystemState>();

            d.name = "StartExp";
            d.param = this;
            d.condition = ShouldStartExperiment;
            d.actionProc = StartExperiment;

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

                if (OS_RESULT_FAILED(this->_queue.Pop(experimentType, InfiniteTimeout)))
                {
                    return;
                }

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
                    break;
                }

                experiment->Run();
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
        }
    }
}
