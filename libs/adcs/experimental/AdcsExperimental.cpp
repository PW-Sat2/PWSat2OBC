#include "AdcsExperimental.hpp"
#include "logger/logger.h"
#include "system.h"

using namespace std::chrono_literals;

using adcs::AdcsExperimental;

AdcsExperimental::AdcsExperimental()
    : taskHandle(nullptr), //
      currentMode(AdcsExperimentMode::Disabled)
{
}

OSResult AdcsExperimental::Initialize()
{
#if 0
    // TODO enable this at experimental adcs integration
    return System::CreateTask(TaskEntry, "ADCSTask", 1024, this, TaskPriority::P4, &this->taskHandle);
#else
    return OSResult::Success;
#endif
}

void AdcsExperimental::HandleCommand()
{
    switch (this->currentMode.load())
    {
        default:
        case AdcsExperimentMode::Disabled:
            break;
        case AdcsExperimentMode::Detumbling:
            // TODO handle single detumbling iteration
            break;
        case AdcsExperimentMode::SunPointing:
            // TODO handle single sun pointing iteration
            break;
    }
}

std::chrono::milliseconds AdcsExperimental::GetIterationtTime()
{
    switch (this->currentMode.load())
    {
        default:
        case AdcsExperimentMode::Disabled:
            return 1000h;
        case AdcsExperimentMode::Detumbling:
            // TODO update this during experimental detumbling integration
            return 1000h;
        case AdcsExperimentMode::SunPointing:
            // TODO update this during experimental sun pointing integration
            return 1000h;
    }
}

void AdcsExperimental::TaskEntry(void* arg)
{
    auto context = static_cast<AdcsExperimental*>(arg);
    for (;;)
    {
        LOGF(LOG_LEVEL_TRACE, "[ADCS] Running ADCS loop. Mode: %d", static_cast<int>(context->currentMode.load()));
        System::SleepTask(context->GetIterationtTime());
    }
}

OSResult AdcsExperimental::EnableSunPointing()
{
    this->currentMode = AdcsExperimentMode::SunPointing;
    return OSResult::Success;
}

OSResult AdcsExperimental::DisableSunPointing()
{
    return TurnOff();
}

OSResult AdcsExperimental::EnableDetumbling()
{
    this->currentMode = AdcsExperimentMode::Detumbling;
    return OSResult::Success;
}

OSResult AdcsExperimental::DisableDetumbling()
{
    return TurnOff();
}

OSResult AdcsExperimental::TurnOff()
{
    this->currentMode = AdcsExperimentMode::Disabled;
    return OSResult::Success;
}
