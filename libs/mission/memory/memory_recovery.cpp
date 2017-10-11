#include "memory_recovery.hpp"

namespace mission
{
    MemoryRecoveryTask::MemoryRecoveryTask(RecoveryProc performRecovery) : _performRecovery(performRecovery)
    {
    }

    ActionDescriptor<SystemState> MemoryRecoveryTask::BuildAction()
    {
        ActionDescriptor<SystemState> d;

        d.name = "Memory recovery";
        d.condition = nullptr;
        d.param = reinterpret_cast<void*>(this->_performRecovery);
        d.actionProc = Action;

        return d;
    }

    void MemoryRecoveryTask::Action(SystemState& /*state*/, void* param)
    {
        auto p = reinterpret_cast<RecoveryProc>(param);

        p();
    }
}
