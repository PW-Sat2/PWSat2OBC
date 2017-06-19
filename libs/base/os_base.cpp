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

ReaderWriterLock::ReaderWriterLock()
    : resourceSemaphore(System::CreateBinarySemaphore()),  //
      readCountSemaphore(System::CreateBinarySemaphore()), //
      serviceSemaphore(System::CreateBinarySemaphore()),   //
      readCount(0),                                        //
      isWriterLockAcquired(false)                          //
{
}

OSResult ReaderWriterLock::Initialize()
{
    OSResult result;

    result = System::GiveSemaphore(resourceSemaphore);
    if (!OS_RESULT_SUCCEEDED(result))
    {
        return result;
    }

    result = System::GiveSemaphore(readCountSemaphore);
    if (!OS_RESULT_SUCCEEDED(result))
    {
        return result;
    }

    return System::GiveSemaphore(serviceSemaphore);
}

bool ReaderWriterLock::AcquireReaderLock(std::chrono::milliseconds timeout)
{
    if (!OS_RESULT_SUCCEEDED(System::TakeSemaphore(serviceSemaphore, timeout)))
    {
        return false;
    }

    if (!OS_RESULT_SUCCEEDED(System::TakeSemaphore(readCountSemaphore, timeout)))
    {
        System::GiveSemaphore(serviceSemaphore);
        return false;
    }

    if (readCount == 0)
    {
        if (!OS_RESULT_SUCCEEDED(System::TakeSemaphore(resourceSemaphore, timeout)))
        {
            System::GiveSemaphore(serviceSemaphore);
            System::GiveSemaphore(readCountSemaphore);
            return false;
        }
    }

    readCount++;

    System::GiveSemaphore(serviceSemaphore);
    System::GiveSemaphore(readCountSemaphore);

    return true;
}

bool ReaderWriterLock::AcquireWriterLock(std::chrono::milliseconds timeout)
{
    if (!OS_RESULT_SUCCEEDED(System::TakeSemaphore(serviceSemaphore, timeout)))
    {
        return false;
    }

    if (!OS_RESULT_SUCCEEDED(System::TakeSemaphore(resourceSemaphore, timeout)))
    {
        System::GiveSemaphore(serviceSemaphore);
        return false;
    }

    isWriterLockAcquired = true;

    System::GiveSemaphore(serviceSemaphore);

    return true;
}

bool ReaderWriterLock::ReleaseReaderLock(std::chrono::milliseconds timeout)
{
    if (!OS_RESULT_SUCCEEDED(System::TakeSemaphore(readCountSemaphore, timeout)))
    {
        return false;
    }

    if (readCount > 0)
    {
        readCount--;
    }

    if (readCount == 0)
    {
        System::GiveSemaphore(resourceSemaphore);
    }

    System::GiveSemaphore(readCountSemaphore);

    return true;
}

bool ReaderWriterLock::ReleaseWriterLock()
{
    if (!isWriterLockAcquired)
    {
        return false;
    }

    isWriterLockAcquired = false;

    System::GiveSemaphore(resourceSemaphore);

    return true;
}

bool ReaderWriterLock::IsReaderLockAcquired() const
{
    if (!OS_RESULT_SUCCEEDED(System::TakeSemaphore(readCountSemaphore, InfiniteTimeout)))
    {
        return true;
    }

    auto isAcquired = readCount > 0;

    System::GiveSemaphore(readCountSemaphore);

    return isAcquired;
}

bool ReaderWriterLock::IsWriterLockAcquired() const
{
    return isWriterLockAcquired;
}

ReaderLock::ReaderLock(ReaderWriterLock& readerWriterLock, std::chrono::milliseconds timeout)
    : readerWriterLock(readerWriterLock), timeout(timeout), taken(readerWriterLock.AcquireReaderLock(timeout))
{
}

ReaderLock::~ReaderLock()
{
    if (!taken)
    {
        return;
    }

    readerWriterLock.ReleaseReaderLock(timeout);
}

WriterLock::WriterLock(ReaderWriterLock& readerWriterLock, std::chrono::milliseconds timeout)
    : readerWriterLock(readerWriterLock), taken(readerWriterLock.AcquireWriterLock(timeout))
{
}

WriterLock::~WriterLock()
{
    if (!taken)
    {
        return;
    }

    readerWriterLock.ReleaseWriterLock();
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
