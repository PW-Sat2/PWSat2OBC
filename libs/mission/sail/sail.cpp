#include "mission/sail.hpp"
#include "state/struct.h"

using namespace std::chrono_literals;

namespace mission
{
    OpenSailTask::StepDescription OpenSailTask::Steps[] = {
        {&EnableMainThermalKnife, 0s},                                     //
        {&EnableMainBurnSwitch, 2min},                                     //
        {&DisableMainThermalKnife, 0s},                                    //
        {&EnableRedundantThermalKnife, 0s},                                //
        {&EnableRedundantBurnSwitch, 2min},                                //
        {&DisableRedundantThermalKnife, std::chrono::milliseconds::max()}, //
    };

    OpenSailTask::OpenSailTask(services::power::IPowerControl& power) : _power(power), _step(0), _nextStepAfter(0)
    {
    }

    UpdateDescriptor<SystemState> OpenSailTask::BuildUpdate()
    {
        UpdateDescriptor<SystemState> update;

        update.name = "Sail: Control";
        update.param = this;
        update.updateProc = Update;

        return update;
    }

    ActionDescriptor<SystemState> OpenSailTask::BuildAction()
    {
        ActionDescriptor<SystemState> action;

        action.name = "Sail: Open";
        action.param = this;
        action.actionProc = Action;
        action.condition = Condition;

        return action;
    }

    UpdateResult OpenSailTask::Update(SystemState& state, void* /*param*/)
    {
        if (state.PersistentState.Get<state::SailState>().CurrentState() != state::SailOpeningState::Waiting)
        {
            return UpdateResult::Ok;
        }

        if (state.Time >= 40 * 24h)
        {
            state.PersistentState.Set(state::SailState(state::SailOpeningState::Opening));

            return UpdateResult::Ok;
        }

        return UpdateResult::Ok;
    }

    bool OpenSailTask::Condition(const SystemState& state, void* param)
    {
        auto This = static_cast<OpenSailTask*>(param);

        if (state.PersistentState.Get<state::SailState>().CurrentState() != state::SailOpeningState::Opening)
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

    void OpenSailTask::Action(SystemState& state, void* param)
    {
        auto This = static_cast<OpenSailTask*>(param);

        while (true)
        {
            This->_step++;
            auto& step = Steps[This->_step - 1];

            step.Action(This);

            if (step.AfterStepDelay != decltype(step.AfterStepDelay)::zero())
            {
                This->_nextStepAfter = state.Time + step.AfterStepDelay;
                break;
            }
        }
    }

    void OpenSailTask::EnableMainThermalKnife(OpenSailTask* This)
    {
        This->_power.MainThermalKnife(true);
    }

    void OpenSailTask::DisableMainThermalKnife(OpenSailTask* This)
    {
        This->_power.MainThermalKnife(false);
    }

    void OpenSailTask::EnableRedundantThermalKnife(OpenSailTask* This)
    {
        This->_power.RedundantThermalKnife(true);
    }

    void OpenSailTask::DisableRedundantThermalKnife(OpenSailTask* This)
    {
        This->_power.RedundantThermalKnife(false);
    }

    void OpenSailTask::EnableMainBurnSwitch(OpenSailTask* This)
    {
        This->_power.EnableMainSailBurnSwitch();
    }

    void OpenSailTask::EnableRedundantBurnSwitch(OpenSailTask* This)
    {
        This->_power.EnableRedundantSailBurnSwitch();
    }
}
