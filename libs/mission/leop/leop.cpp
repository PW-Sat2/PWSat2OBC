#include "experiment/leop/leop.hpp"
#include <cstdint>
#include "leop_task.hpp"
#include "logger/logger.h"
#include "mission/base.hpp"

namespace mission
{
    namespace leop
    {
        LEOPTask::LEOPTask(std::pair<::experiments::IExperimentController&, services::time::ICurrentTime&> parameters)
            : _experimentController(parameters.first), _timeProvider(parameters.second)
        {
        }

        void LEOPTask::AutostartEnable()
        {
            _isStartAllowed = true;
        }

        void LEOPTask::AutostartDisable()
        {
            _isStartAllowed = false;
        }

        mission::ActionDescriptor<SystemState> LEOPTask::BuildAction()
        {
            mission::ActionDescriptor<SystemState> action;
            action.name = "Start LEOP experiment";
            action.param = this;
            action.condition = Condition;
            action.actionProc = Action;
            return action;
        }

        bool LEOPTask::Condition(const SystemState& /*state*/, void* param)
        {
            auto This = reinterpret_cast<LEOPTask*>(param);

            bool isTimeForStart = false;

            if (!This->_taskStartTime.HasValue)
            {
                This->_taskStartTime = This->_timeProvider.GetCurrentTime();
            }
            else
            {
                auto currentTime = This->_timeProvider.GetCurrentTime();
                if (!currentTime.HasValue)
                {
                    return false;
                }

                isTimeForStart = currentTime.Value - This->_taskStartTime.Value > ExperimentStartDelay;
            }

            return This->_isStartAllowed && isTimeForStart;
        }

        void LEOPTask::Action(SystemState& /*state*/, void* param)
        {
            LOG(LOG_LEVEL_INFO, "[leop] Starting Leop Experiment");
            auto This = reinterpret_cast<LEOPTask*>(param);
            This->_experimentController.RequestExperiment(experiment::leop::LaunchAndEarlyOrbitPhaseExperiment::Code);
            This->_isStartAllowed = false;
        }
    }
}
