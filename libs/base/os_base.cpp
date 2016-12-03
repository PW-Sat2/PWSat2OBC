#include "os.h"

TakeSemaphore::TakeSemaphore(OSSemaphoreHandle semaphore, OSTaskTimeSpan timeout) : _semaphore(semaphore)
{
    if (OS_RESULT_SUCCEEDED(System::TakeSemaphore(this->_semaphore, timeout)))
    {
        this->_taken = true;
    }
    else
    {
        this->_taken = false;
    }
}

TakeSemaphore::~TakeSemaphore()
{
    if (this->_taken)
    {
        System::GiveSemaphore(this->_semaphore);
    }
}

bool TakeSemaphore::operator()()
{
    return this->_taken;
}
