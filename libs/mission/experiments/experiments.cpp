#include "experiments.hpp"

using experiments::ExperimentController;

namespace mission
{
    namespace experiments
    {
        MissionExperimentComponent::MissionExperimentComponent(ExperimentController& experimentController)
            : _experimentController(experimentController), //
              _actions("Experiment", KickExperimentAction(), StartExperimentAction())
        {
        }

        mission::ActionDescriptor<SystemState> MissionExperimentComponent::BuildAction()
        {
            return this->_actions.BuildAction();
        }

        mission::UpdateDescriptor<SystemState> MissionExperimentComponent::BuildUpdate()
        {
            UpdateDescriptor<SystemState> d;

            d.name = "ExperimentState";
            d.param = this;
            d.updateProc = UpdateSystemState;

            return d;
        }

        mission::UpdateResult MissionExperimentComponent::UpdateSystemState(SystemState& state, void* param)
        {
            auto This = reinterpret_cast<MissionExperimentComponent*>(param);

            state.Experiment = This->_experimentController.CurrentState();

            return mission::UpdateResult::Ok;
        }

        mission::ActionDescriptor<SystemState> MissionExperimentComponent::StartExperimentAction()
        {
            auto d = mission::ActionDescriptor<SystemState>();

            d.name = "StartExp";
            d.param = this;
            d.condition = ShouldStartExperiment;
            d.actionProc = StartExperiment;

            return d;
        }

        mission::ActionDescriptor<SystemState> MissionExperimentComponent::KickExperimentAction()
        {
            auto d = mission::ActionDescriptor<SystemState>();

            d.name = "KickExp";
            d.param = this;
            d.condition = ShouldKickExperiment;
            d.actionProc = KickExperiment;

            return d;
        }

        bool MissionExperimentComponent::ShouldStartExperiment(const SystemState& state, void* param)
        {
            UNREFERENCED_PARAMETER(state);

            auto This = reinterpret_cast<MissionExperimentComponent*>(param);

            if (This->_experimentController.InProgress())
                return false;

            return This->_experimentController.IsExperimentRequested();
        }

        void MissionExperimentComponent::StartExperiment(SystemState& state, void* param)
        {
            UNREFERENCED_PARAMETER(state);

            auto This = reinterpret_cast<MissionExperimentComponent*>(param);

            This->_experimentController.StartExperiment();
        }

        bool MissionExperimentComponent::ShouldKickExperiment(const SystemState& state, void* param)
        {
            UNREFERENCED_PARAMETER(state);

            auto This = reinterpret_cast<MissionExperimentComponent*>(param);

            return This->_experimentController.InProgress();
        }

        void MissionExperimentComponent::KickExperiment(SystemState& state, void* param)
        {
            UNREFERENCED_PARAMETER(state);

            auto This = reinterpret_cast<MissionExperimentComponent*>(param);

            This->_experimentController.NotifyLoopIterationStart();
        }
    }
}
