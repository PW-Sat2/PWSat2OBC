#include "mission/sail.hpp"
#include "logger/logger.h"
#include "state/struct.h"

using namespace std::chrono_literals;

namespace mission
{
    OpenSailTask::StepProc OpenSailTask::Steps[] = {
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
        &DisableMainThermalKnife,      //
        &Delay100ms,                   //
        &DisableMainThermalKnife,      //
        &Delay100ms,                   //
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
        &Delay100ms,                   //
        &DisableRedundantThermalKnife, //
    };

    OpenSailTask::OpenSailTask(services::power::IPowerControl& power)
        : _power(power), _step(0), _nextStepAfter(0), _openOnNextMissionLoop(false)
    {
    }

    void OpenSailTask::OpenSail()
    {
        this->_openOnNextMissionLoop = true;
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

    UpdateResult OpenSailTask::Update(SystemState& state, void* param)
    {
        auto This = static_cast<OpenSailTask*>(param);

        state::SailState sailState;
        if (!state.PersistentState.Get(sailState))
        {
            LOG(LOG_LEVEL_ERROR, "Can't get sail state");
            return UpdateResult::Failure;
        }

        auto currentState = sailState.CurrentState();

        auto open = [&state, This]() {
            state.PersistentState.Set(state::SailState(state::SailOpeningState::Opening));
            This->_step = 0;
            This->_nextStepAfter = 0s;
        };

        auto explicitOpen = This->_openOnNextMissionLoop.exchange(false);

        switch (currentState)
        {
            case state::SailOpeningState::Opening:
            {
                if (explicitOpen && !This->InProgress())
                {
                    open();
                    break;
                }
            }
            break;

            case state::SailOpeningState::OpeningStopped:
            {
                if (explicitOpen)
                {
                    open();
                    break;
                }
            }
            break;

            case state::SailOpeningState::Waiting:
            {
                if (state.Time >= 40 * 24h)
                {
                    open();
                    break;
                }

                if (explicitOpen)
                {
                    open();
                    break;
                }
            }
            break;

            case state::SailOpeningState::Disabled:
            {
                if (explicitOpen)
                {
                    open();
                    break;
                }

                LOG(LOG_LEVEL_WARNING, "[sail] Sail deployment disabled");
                return UpdateResult::Warning;
            }
            break;
        }

        return UpdateResult::Ok;
    }

    bool OpenSailTask::Condition(const SystemState& state, void* param)
    {
        auto This = static_cast<OpenSailTask*>(param);

        state::SailState sailState;
        if (!state.PersistentState.Get(sailState))
        {
            LOG(LOG_LEVEL_ERROR, "Can't get sail state");
            return false;
        }

        if (sailState.CurrentState() != state::SailOpeningState::Opening)
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

        while (This->_step < StepsCount)
        {
            LOGF(LOG_LEVEL_INFO, "[sail] Performing step %d", This->_step);

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

    void OpenSailTask::Delay100ms(OpenSailTask* /*This*/, SystemState& /*state*/)
    {
        System::SleepTask(100ms);
    }

    void OpenSailTask::WaitFor2mins(OpenSailTask* This, SystemState& state)
    {
        This->_nextStepAfter = state.Time + 2min;
    }

    void OpenSailTask::EnableMainThermalKnife(OpenSailTask* This, SystemState& /*state*/)
    {
        This->_power.MainThermalKnife(true);
    }

    void OpenSailTask::DisableMainThermalKnife(OpenSailTask* This, SystemState& /*state*/)
    {
        This->_power.MainThermalKnife(false);
    }

    void OpenSailTask::EnableRedundantThermalKnife(OpenSailTask* This, SystemState& /*state*/)
    {
        This->_power.RedundantThermalKnife(true);
    }

    void OpenSailTask::DisableRedundantThermalKnife(OpenSailTask* This, SystemState& /*state*/)
    {
        This->_power.RedundantThermalKnife(false);
    }

    void OpenSailTask::EnableMainBurnSwitch(OpenSailTask* This, SystemState& /*state*/)
    {
        This->_power.EnableMainSailBurnSwitch();
    }

    void OpenSailTask::EnableRedundantBurnSwitch(OpenSailTask* This, SystemState& /*state*/)
    {
        This->_power.EnableRedundantSailBurnSwitch();
    }

    StopSailDeploymentTask::StopSailDeploymentTask(uint8_t)
    {
    }

    mission::ActionDescriptor<SystemState> StopSailDeploymentTask::BuildAction()
    {
        mission::ActionDescriptor<SystemState> action;
        action.name = "Stop sail deployment";
        action.param = this;
        action.condition = Condition;
        action.actionProc = Action;
        return action;
    }

    void StopSailDeploymentTask::DisableDeployment()
    {
        this->shouldDisable = true;
    }

    bool StopSailDeploymentTask::Condition(const SystemState& state, void* param)
    {
        state::SailState sailState;
        if (!state.PersistentState.Get(sailState))
        {
            LOG(LOG_LEVEL_ERROR, "[sail] Can't get sail state");
            return false;
        }

        auto This = reinterpret_cast<StopSailDeploymentTask*>(param);
        auto currentState = sailState.CurrentState();

        return This->shouldDisable && !state.SailOpened && currentState != state::SailOpeningState::Opening &&
            currentState != state::SailOpeningState::Disabled;
    }

    void StopSailDeploymentTask::Action(SystemState& state, void*)
    {
        state::SailState sailState;
        if (!state.PersistentState.Get(sailState))
        {
            LOG(LOG_LEVEL_ERROR, "[sail] Can't get sail state");
            return;
        }

        if (!state.PersistentState.Set(state::SailState(state::SailOpeningState::Disabled)))
        {
            LOG(LOG_LEVEL_ERROR, "[sail] Can't set sail state");
        }
    }
}
