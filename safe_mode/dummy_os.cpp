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
