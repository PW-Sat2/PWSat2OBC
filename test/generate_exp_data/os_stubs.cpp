#include "base/os.h"

void System::SleepTask(std::chrono::milliseconds)
{
}

OSResult System::TakeSemaphore(OSSemaphoreHandle, std::chrono::milliseconds)
{
    return OSResult::Success;
}

OSResult System::GiveSemaphore(OSSemaphoreHandle)
{
    return OSResult::Success;
}

std::chrono::milliseconds System::GetUptime()
{
    return std::chrono::milliseconds::zero();
}

OSEventGroupHandle System::CreateEventGroup()
{
    return 0;
}

OSEventBits System::EventGroupGetBits(OSEventGroupHandle)
{
    return 0;
}

OSEventBits System::EventGroupSetBits(OSEventGroupHandle, const OSEventBits)
{
    return 0;
}

OSEventBits System::EventGroupSetBitsISR(OSEventGroupHandle, const OSEventBits)
{
    return 0;
}

OSEventBits System::EventGroupClearBits(OSEventGroupHandle, const OSEventBits)
{
    return 0;
}

OSEventBits System::EventGroupWaitForBits(OSEventGroupHandle, const OSEventBits, bool, bool, const std::chrono::milliseconds)
{
    return 0;
}
