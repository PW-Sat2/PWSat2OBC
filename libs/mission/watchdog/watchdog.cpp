#include "watchdog.hpp"
#include "eps/eps.h"

using devices::eps::EPSDriver;

namespace mission
{
    WatchdogTask::WatchdogTask(devices::eps::EPSDriver& eps) : _eps(eps)
    {
    }

    ActionDescriptor<SystemState> WatchdogTask::BuildAction()
    {
        ActionDescriptor<SystemState> action;

        action.name = "Watchdog";
        action.condition = nullptr;
        action.actionProc = Kick;
        action.param = this;

        return action;
    }

    void WatchdogTask::Kick(SystemState& /*state*/, void* param)
    {
        auto This = reinterpret_cast<WatchdogTask*>(param);

        This->_eps.ResetWatchdog(EPSDriver::Controller::A);
        This->_eps.ResetWatchdog(EPSDriver::Controller::B);
    }
}
