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

__attribute__((optimize("O0"))) void wait_100ms_spinloop() {
    // about 100 ms spin-loop delay
    // 0.3666666 second = 100000 ticks
    // 0.1 sec = 27272 ticks

    volatile uint32_t timeout = 27272;
    while(timeout--) {};
}

Timeout::Timeout(std::chrono::milliseconds timeout) : milliseconds_left(timeout.count())
{
}

bool Timeout::Expired()
{
    wait_100ms_spinloop();

    milliseconds_left -= 100;
    return milliseconds_left <= 0;
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

CriticalSection::CriticalSection()
{
    System::EnterCritical();
}

CriticalSection::~CriticalSection()
{
    System::LeaveCritical();
}
