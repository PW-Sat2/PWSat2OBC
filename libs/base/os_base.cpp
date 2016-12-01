#include <FreeRTOS.h>

#include "os.h"

Lock::Lock(OSSemaphoreHandle semaphore, OSTaskTimeSpan timeout) : _semaphore(semaphore)
{
    this->_taken = OS_RESULT_SUCCEEDED(System::TakeSemaphore(this->_semaphore, timeout));
}

Lock::~Lock()
{
    if (this->_taken)
    {
        System::GiveSemaphore(this->_semaphore);
    }
}

bool Lock::operator()()
{
    return this->_taken;
}


constexpr std::uint32_t MilisecondsToTicks(std::uint32_t miliseconds)
{
    return pdMS_TO_TICKS(miliseconds);
}

Timeout::Timeout(std::uint32_t timeout) : _expireAt(System::GetTickCount() + MilisecondsToTicks(timeout))
{
}

bool Timeout::Expired()
{
    return System::GetTickCount() >= this->_expireAt;
}
