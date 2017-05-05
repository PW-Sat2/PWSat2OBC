#include "mission/comm.hpp"
#include "logger/logger.h"
#include "state/struct.h"

using namespace std::chrono_literals;

namespace mission
{
    CommTask::CommTask(devices::comm::ITransmitter& transmitter) : _transmitter(transmitter), _idleState(false)
    {
    }

    ActionDescriptor<SystemState> CommTask::BuildAction()
    {
        ActionDescriptor<SystemState> descriptor;
        descriptor.name = "Comm Action";
        descriptor.param = this;
        descriptor.condition = CanLeaveTransmitterStateWhenIdle;
        descriptor.actionProc = ActionProc;
        return descriptor;
    }

    bool CommTask::EnterTransmitterStateWhenIdle(std::chrono::milliseconds turnOffTime)
    {
        LOG(LOG_LEVEL_INFO, "Entering idle state");

        auto result = this->_transmitter.SetTransmitterStateWhenIdle(devices::comm::IdleState::On);
        if (result)
        {
            this->_idleState = true;
            this->_idleStateTurnOffTime = turnOffTime;
        }

        return result;
    }

    bool CommTask::LeaveTransmitterStateWhenIdle()
    {
        LOG(LOG_LEVEL_INFO, "Leaving idle state");

        auto result = this->_transmitter.SetTransmitterStateWhenIdle(devices::comm::IdleState::Off);
        if (result)
        {
            this->_idleState = false;
        }

        return result;
    }

    bool CommTask::CanLeaveTransmitterStateWhenIdle(const SystemState& state, void* param)
    {
        auto This = static_cast<CommTask*>(param);

        return This->_idleState && This->_idleStateTurnOffTime <= state.Time;
    }

    void CommTask::ActionProc(const SystemState& state, void* param)
    {
        UNREFERENCED_PARAMETER(state);

        auto This = static_cast<CommTask*>(param);
        This->LeaveTransmitterStateWhenIdle();
    }
}
