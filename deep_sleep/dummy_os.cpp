#include "base/os.h"

void System::EnterCritical()
{
}

void System::LeaveCritical()
{
}

void System::SleepTask(std::chrono::milliseconds)
{
}

void System::Yield()
{
}

std::chrono::milliseconds System::GetUptime()
{
    return std::chrono::milliseconds::zero();
}
