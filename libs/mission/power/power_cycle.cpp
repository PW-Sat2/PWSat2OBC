#include "power_cycle.hpp"
#include "power/power.h"
#include "logger/logger.h"

using namespace std::chrono_literals;

namespace mission
{
    namespace power
    {
        PeriodicPowerCycleTask::PeriodicPowerCycleTask(
            std::tuple<services::power::IPowerControl&, IScrubbingStatus&> args)
            : _power(std::get<0>(args)), _scrubbingStatus(std::get<1>(args))
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

            if (timeSinceBoot < PowerCycleTime)
            {
                return false;
            }

            if (This->_scrubbingStatus.BootloaderInProgress())
            {
                return false;
            }

            if (This->_scrubbingStatus.PrimarySlotsInProgress())
            {
                return false;
            }

            if (This->_scrubbingStatus.FailsafeSlotsInProgress())
            {
                return false;
            }

            if (This->_scrubbingStatus.SafeModeInProgress())
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

        void PeriodicPowerCycleTask::TimeChanged(std::chrono::milliseconds timeCorrection)
        {
            if (this->_bootTime.HasValue)
            {
                this->_bootTime.Value += timeCorrection;
            }
        }
    }
}
