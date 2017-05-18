#include "os.h"

Lock::Lock(OSSemaphoreHandle semaphore, std::chrono::milliseconds timeout) : _semaphore(semaphore)
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

Timeout::Timeout(std::chrono::milliseconds timeout) : _expireAt(System::GetUptime() + timeout)
{
}

bool Timeout::Expired()
{
    return System::GetUptime() >= this->_expireAt;
}

EventGroup::EventGroup() : _handle(nullptr)
{
}

OSResult EventGroup::Initialize()
{
    this->_handle = System::CreateEventGroup();

    if (this->_handle == 0)
    {
        return OSResult::NotEnoughMemory;
    }

    return OSResult::Success;
}

void EventGroup::Set(OSEventBits bits)
{
    System::EventGroupSetBits(this->_handle, bits);
}

void EventGroup::SetISR(OSEventBits bits)
{
    System::EventGroupSetBitsISR(this->_handle, bits);
}

void EventGroup::Clear(OSEventBits bits)
{
    System::EventGroupClearBits(this->_handle, bits);
}

OSEventBits EventGroup::WaitAny(OSEventBits bits, bool clearOnExit, std::chrono::milliseconds timeout)
{
    return System::EventGroupWaitForBits(this->_handle, bits, false, clearOnExit, timeout);
}

OSEventBits EventGroup::WaitAll(OSEventBits bits, bool clearOnExit, std::chrono::milliseconds timeout)
{
    return System::EventGroupWaitForBits(this->_handle, bits, true, clearOnExit, timeout);
}

bool EventGroup::IsSet(OSEventBits bit)
{
    auto f = System::EventGroupGetBits(this->_handle);

    return has_flag(f, bit);
}
