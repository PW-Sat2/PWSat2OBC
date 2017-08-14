#include "mission/comm_wdog.hpp"

namespace mission
{
    CommWdogTask::CommWdogTask(devices::comm::ICommHardwareObserver* commObserver) : observer(commObserver)
    {
    }

    ActionDescriptor<SystemState> CommWdogTask::BuildAction()
    {
        ActionDescriptor<SystemState> descriptor;
        descriptor.name = "Comm Watchdog";
        descriptor.param = this->observer;
        descriptor.condition = ConditionProc;
        descriptor.actionProc = ActionProc;
        return descriptor;
    }

    bool CommWdogTask::ConditionProc(const SystemState& /*state*/, void* param)
    {
        return param != nullptr;
    }

    void CommWdogTask::ActionProc(SystemState& /*state*/, void* param)
    {
        auto driver = static_cast<devices::comm::ICommHardwareObserver*>(param);
        driver->WaitForComLoop();
    }
}
