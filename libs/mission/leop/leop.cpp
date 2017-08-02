#include "experiment/leop/leop.hpp"
#include <cstdint>
#include <cstring>
#include "leop_task.hpp"
#include "logger/logger.h"
#include "mission/base.hpp"
#include "system.h"

using namespace std::chrono_literals;

namespace mission
{
    namespace leop
    {
        LEOPTask::LEOPTask(::experiments::ExperimentController& experimentController) : _experimentController(experimentController)
        {
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
            return This->_isEnabled;
        }

        void LEOPTask::Action(SystemState& /*state*/, void* param)
        {
            LOG(LOG_LEVEL_INFO, "[leop] Starting Leop Experiment");
            auto This = reinterpret_cast<LEOPTask*>(param);
            This->_experimentController.RequestExperiment(experiment::leop::LaunchAndEarlyOrbitPhaseExperiment::Code);
            This->_isEnabled = false;
        }
    }
}
