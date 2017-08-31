#include "power_cycle.hpp"
#include "power/power.h"

using namespace std::chrono_literals;

namespace mission
{
    namespace power
    {
        PeriodicPowerCycleTask::PeriodicPowerCycleTask(services::power::IPowerControl& power) : _power(power)
        {
        }

        ActionDescriptor<SystemState> PeriodicPowerCycleTask::BuildAction()
        {
            ActionDescriptor<SystemState> action;

            action.name = "Periodic power cycle";
            action.actionProc = Action;
            action.condition = Condition;
            action.param = this;

            return action;
        }

        bool PeriodicPowerCycleTask::Condition(const SystemState& state, void* param)
        {
            auto This = static_cast<PeriodicPowerCycleTask*>(param);

            if (!This->_bootTime.HasValue)
            {
                auto currentTime = state.Time;
                This->_bootTime = Some(currentTime);
                return false;
            }

            auto timeSinceBoot = state.Time - This->_bootTime.Value;

            if (timeSinceBoot < 23h)
            {
                return false;
            }

            return true;
        }

        void PeriodicPowerCycleTask::Action(SystemState& /*state*/, void* param)
        {
            auto This = static_cast<PeriodicPowerCycleTask*>(param);

            LOG(LOG_LEVEL_WARNING, "[power_cycle] Triggering periodic power cycle");
            This->_power.PowerCycle();
        }
    }
}
