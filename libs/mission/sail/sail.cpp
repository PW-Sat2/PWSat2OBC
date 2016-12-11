#include "mission/sail.hpp"
#include "eps/eps.h"
#include "state/struct.h"

namespace mission
{
    SailTask::SailTask(bool initial) : state(initial)
    {
    }

    ActionDescriptor<SystemState> SailTask::BuildAction()
    {
        ActionDescriptor<SystemState> descriptor;
        descriptor.name = "Open Sail Action";
        descriptor.param = this;
        descriptor.condition = CanOpenSail;
        descriptor.actionProc = OpenSail;
        return descriptor;
    }

    UpdateDescriptor<SystemState> SailTask::BuildUpdate()
    {
        UpdateDescriptor<SystemState> descriptor;
        descriptor.name = "Update Sail State";
        descriptor.updateProc = UpdateProc;
        descriptor.param = this;
        return descriptor;
    }

    UpdateResult SailTask::UpdateProc(SystemState& state, void* param)
    {
        auto This = static_cast<SailTask*>(param);
        state.SailOpened = This->state;
        return UpdateResult::UpdateOK;
    }

    bool SailTask::CanOpenSail(const SystemState& state, void* param)
    {
        UNREFERENCED_PARAMETER(param);

        const TimeSpan t = TimeSpanFromHours(40);
        if (TimeSpanLessThan(state.Time, t))
        {
            return false;
        }

        if (state.SailOpened)
        {
            return false;
        }

        return true;
    }

    void SailTask::OpenSail(const SystemState& state, void* param)
    {
        auto This = static_cast<SailTask*>(param);
        UNREFERENCED_PARAMETER(state);
        This->state = true;
        EpsOpenSail();
    }
}
