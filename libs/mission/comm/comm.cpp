#include "mission/comm.hpp"
#include "logger/logger.h"
#include "state/struct.h"
#include "telecommunication/downlink.h"

using namespace std::chrono_literals;

namespace mission
{
    CommTask::CommTask(devices::comm::ITransmitter& transmitter)
        : _transmitterStateWhenIdleSem(System::CreateBinarySemaphore()), _transmitter(transmitter), _idleState(false)
    {
    }

    ActionDescriptor<SystemState> CommTask::BuildAction()
    {
        System::GiveSemaphore(this->_transmitterStateWhenIdleSem);

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

        Lock lock(this->_transmitterStateWhenIdleSem, InfiniteTimeout);
        if (!lock())
        {
            LOG(LOG_LEVEL_ERROR, "Cannot enter idle state - lock failed");
            return false;
        }

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

        Lock lock(this->_transmitterStateWhenIdleSem, InfiniteTimeout);
        if (!lock())
        {
            LOG(LOG_LEVEL_ERROR, "Cannot leave idle state - lock failed");
            return false;
        }

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

    void CommTask::ActionProc(SystemState& state, void* param)
    {
        UNREFERENCED_PARAMETER(state);

        auto This = static_cast<CommTask*>(param);
        This->LeaveTransmitterStateWhenIdle();
    }

    SendMessageTask::SendMessageTask(devices::comm::ITransmitter& transmitter) : _transmitter(transmitter)
    {
    }

    mission::ActionDescriptor<SystemState> SendMessageTask::BuildAction()
    {
        mission::ActionDescriptor<SystemState> action;

        action.name = "Send message";
        action.param = this;
        action.condition = Condition;
        action.actionProc = Action;

        return action;
    }

    bool SendMessageTask::Condition(const SystemState& state, void* param)
    {
        auto This = static_cast<SendMessageTask*>(param);

        if (!state.AntennaState.IsDeployed())
        {
            return false;
        }

        state::MessageState settings;

        if (!state.PersistentState.Get(settings))
        {
            LOG(LOG_LEVEL_WARNING, "[periodic msg] Unable to get message settings");
            return false;
        }

        if (!This->_lastSentAt.HasValue)
        {
            return true;
        }

        return This->_lastSentAt.Value + settings.Interval() <= state.Time;
    }

    void SendMessageTask::Action(SystemState& state, void* param)
    {
        auto This = static_cast<SendMessageTask*>(param);

        state::MessageState settings;

        if (!state.PersistentState.Get(settings))
        {
            LOG(LOG_LEVEL_WARNING, "[periodic msg] Unable to get message settings");
            return;
        }

        telecommunication::downlink::DownlinkFrame frame(telecommunication::downlink::DownlinkAPID::PeriodicMessage, 0);
        frame.PayloadWriter().WriteArray(settings.Message());

        for (auto i = 0; i < settings.RepeatCount(); i++)
        {
            This->_transmitter.SendFrame(frame.Frame());
        }

        This->_lastSentAt = Some(state.Time);
    }
}
