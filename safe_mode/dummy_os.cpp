#include "base/os.h"

OSSemaphoreHandle System::CreateBinarySemaphore(std::uint8_t)
{
    return reinterpret_cast<OSSemaphoreHandle>(1);
}

OSResult System::TakeSemaphore(OSSemaphoreHandle, std::chrono::milliseconds)
{
    return OSResult::Success;
}

OSResult System::GiveSemaphore(OSSemaphoreHandle)
{
    return OSResult::Success;
}

void System::Yield()
{
}

std::chrono::milliseconds System::GetUptime()
{
    return std::chrono::milliseconds::zero();
}

OSEventGroupHandle System::CreateEventGroup()
{
    return reinterpret_cast<OSEventGroupHandle>(1);
}

OSEventBits System::EventGroupSetBits(OSEventGroupHandle /*eventGroup*/, const OSEventBits bitsToChange)
{
    return bitsToChange;
}

OSEventBits System::EventGroupSetBitsISR(OSEventGroupHandle /*eventGroup*/, const OSEventBits bitsToChange)
{
    return bitsToChange;
}

OSEventBits System::EventGroupWaitForBits(OSEventGroupHandle /*eventGroup*/,
    const OSEventBits bitsToWaitFor,
    bool /*waitAll*/,
    bool /*autoReset*/,
    const std::chrono::milliseconds /*timeout*/)
{
    return bitsToWaitFor;
}

OSEventBits System::EventGroupClearBits(OSEventGroupHandle /*eventGroup*/, const OSEventBits /*bitsToChange*/)
{
    return 0;
}

OSEventBits System::EventGroupGetBits(OSEventGroupHandle /*eventGroup*/)
{
    return 0;
}
