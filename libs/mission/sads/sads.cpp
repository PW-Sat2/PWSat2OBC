#include "mission/sads.hpp"
#include "logger/logger.h"
#include "state/struct.h"

using namespace std::chrono_literals;

namespace mission
{
    /*
     *  Send a command sequence to EPS to switch main thermal knives on for 2min:
     *  "enable TKmain LCL",
     *  wait 100ms and
     *  "enable TKmain LCL" to controller A,
     *  wait 100ms,
     *  "enable SADSmain BURN switch",
     *  wait 100ms and
     *  "enable SADSmain BURN switch" to controller A.
     *
     *  for 2min:
     *  "enable TKmain LCL",
     *  wait 100ms and
     *  "enable TKmain LCL" to controller B, wait 100ms,
     *  "enable SADSmain BURN switch",
     *  wait 100ms and
     *  "enable SADSmain BURN switch" to controller B.
     *
     *  T+4min - Send a command sequence to EPS to switch all thermal knife's LCLs off: "disable TKmain LCL" to controller A and "disable
     * TKred LCL" to controller B.
     */

    DeploySolarArrayTask::StepProc DeploySolarArrayTask::Steps[] = {
        &EnableMainThermalKnife,       //
        &Delay100ms,                   //
        &EnableMainThermalKnife,       //
        &Delay100ms,                   //
        &EnableMainBurnSwitch,         //
        &Delay100ms,                   //
        &EnableMainBurnSwitch,         //
                                       //
        &WaitFor2mins,                 //
                                       //
        &EnableRedundantThermalKnife,  //
        &Delay100ms,                   //
        &EnableRedundantThermalKnife,  //
        &Delay100ms,                   //
        &EnableRedundantBurnSwitch,    //
        &Delay100ms,                   //
        &EnableRedundantBurnSwitch,    //
                                       //
        &WaitFor2mins,                 //
                                       //
        &DisableRedundantThermalKnife, //
        &DisableMainThermalKnife,      //
    };

    DeploySolarArrayTask::DeploySolarArrayTask(services::power::IPowerControl& power)
        : _power(power), _step(0), _nextStepAfter(0), _deployOnNextMissionLoop(false), _isDeploying(false)
    {
    }

    void DeploySolarArrayTask::DeploySolarArray()
    {
        this->_deployOnNextMissionLoop = true;
    }

    UpdateDescriptor<SystemState> DeploySolarArrayTask::BuildUpdate()
    {
        UpdateDescriptor<SystemState> update;

        update.name = "Sail: Control";
        update.param = this;
        update.updateProc = Update;

        return update;
    }

    ActionDescriptor<SystemState> DeploySolarArrayTask::BuildAction()
    {
        ActionDescriptor<SystemState> action;

        action.name = "Sail: Open";
        action.param = this;
        action.actionProc = Action;
        action.condition = Condition;

        return action;
    }

    UpdateResult DeploySolarArrayTask::Update(SystemState& /*state*/, void* param)
    {
        auto This = static_cast<DeploySolarArrayTask*>(param);
        auto explicitDeploy = This->_deployOnNextMissionLoop.exchange(false);

        if (This->_step >= StepsCount)
        {
            This->_isDeploying = false;
        }

        if (explicitDeploy && !This->_isDeploying)
        {
            This->_step = 0;
            This->_nextStepAfter = 0s;
            This->_isDeploying = true;
        }

        return UpdateResult::Ok;
    }

    bool DeploySolarArrayTask::Condition(const SystemState& state, void* param)
    {
        auto This = static_cast<DeploySolarArrayTask*>(param);

        if (!This->_isDeploying)
        {
            return false;
        }

        if (state.Time < This->_nextStepAfter)
        {
            return false;
        }

        if (This->_step >= StepsCount)
        {
            return false;
        }

        return true;
    }

    void DeploySolarArrayTask::Action(SystemState& state, void* param)
    {
        auto This = static_cast<DeploySolarArrayTask*>(param);

        while (This->_step < StepsCount)
        {
            LOGF(LOG_LEVEL_INFO, "[sads] Performing step %d", This->_step);

            auto& step = Steps[This->_step];
            This->_step++;

            This->_nextStepAfter = state.Time;

            step(This, state);

            if (This->_nextStepAfter > state.Time)
            {
                break;
            }
        }
    }

    void DeploySolarArrayTask::Delay100ms(DeploySolarArrayTask* /*This*/, SystemState& /*state*/)
    {
        System::SleepTask(100ms);
    }

    void DeploySolarArrayTask::WaitFor2mins(DeploySolarArrayTask* This, SystemState& state)
    {
        This->_nextStepAfter = state.Time + 2min;
    }

    void DeploySolarArrayTask::EnableMainThermalKnife(DeploySolarArrayTask* This, SystemState& /*state*/)
    {
        This->_power.MainThermalKnife(true);
    }

    void DeploySolarArrayTask::DisableMainThermalKnife(DeploySolarArrayTask* This, SystemState& /*state*/)
    {
        This->_power.MainThermalKnife(false);
    }

    void DeploySolarArrayTask::EnableRedundantThermalKnife(DeploySolarArrayTask* This, SystemState& /*state*/)
    {
        This->_power.RedundantThermalKnife(true);
    }

    void DeploySolarArrayTask::DisableRedundantThermalKnife(DeploySolarArrayTask* This, SystemState& /*state*/)
    {
        This->_power.RedundantThermalKnife(false);
    }

    void DeploySolarArrayTask::EnableMainBurnSwitch(DeploySolarArrayTask* This, SystemState& /*state*/)
    {
        This->_power.EnableMainSADSBurnSwitch();
    }

    void DeploySolarArrayTask::EnableRedundantBurnSwitch(DeploySolarArrayTask* This, SystemState& /*state*/)
    {
        This->_power.EnableRedundantSADSBurnSwitch();
    }
}
