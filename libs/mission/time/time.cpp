#include "Include/mission/time.hpp"
#include "state/struct.h"

namespace mission
{
    TimeTask::TimeTask(TimeProvider& timeProvider) : provider(timeProvider)
    {
    }

    UpdateResult TimeTask::UpdateProc(SystemState& state, void* param)
    {
        auto timeProvider = static_cast<TimeProvider*>(param);
        const bool status = TimeGetCurrentTime(timeProvider, &state.Time);
        if (status)
        {
            return UpdateResult::Ok;
        }
        else
        {
            return UpdateResult::Warning;
        }
    }

    UpdateDescriptor<SystemState> TimeTask::BuildUpdate()
    {
        UpdateDescriptor<SystemState> descriptor;
        descriptor.name = "Time State Update";
        descriptor.updateProc = UpdateProc;
        descriptor.param = &this->provider;
        return descriptor;
    }
}
