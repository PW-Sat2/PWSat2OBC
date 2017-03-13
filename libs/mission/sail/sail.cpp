#include "mission/sail.hpp"
#include "state/struct.h"

using namespace std::chrono_literals;

namespace mission
{
    SailTask::SailTask(std::tuple<bool, services::power::IPowerControl&> args) : state(std::get<0>(args)), _powerControl(&std::get<1>(args))
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
        return UpdateResult::Ok;
    }

    bool SailTask::CanOpenSail(const SystemState& state, void* param)
    {
        UNREFERENCED_PARAMETER(param);

        const auto t = 40h;
        if (state.Time < t)
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
        This->_powerControl->OpenSail();
    }
}
