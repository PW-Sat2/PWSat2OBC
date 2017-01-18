#include "Include/mission/time.hpp"
#include "state/struct.h"

using services::time::TimeProvider;

namespace mission
{
    TimeTask::TimeTask(TimeProvider& timeProvider) : provider(timeProvider)
    {
    }

    UpdateResult TimeTask::UpdateProc(SystemState& state, void* param)
    {
        auto timeProvider = static_cast<TimeProvider*>(param);
        Option<std::chrono::milliseconds> currentTime = timeProvider->GetCurrentTime();

        if (currentTime.HasValue)
        {
            state.Time = currentTime.Value;
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
