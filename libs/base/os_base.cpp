
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

Timeout::Timeout(OSTaskTimeSpan timeout) : _expireAt(System::GetUptime() + timeout)
{
}

bool Timeout::Expired()
{
    return System::GetUptime() >= this->_expireAt;
}
