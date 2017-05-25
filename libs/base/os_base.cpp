
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

bool Lock::operator()()
{
    return this->_taken;
}

ReaderWriterLock::ReaderWriterLock()
    : resourceSemaphore(System::CreateBinarySemaphore()),  //
      readCountSemaphore(System::CreateBinarySemaphore()), //
      serviceSemaphore(System::CreateBinarySemaphore()),   //
      readCount(0),                                        //
      isWriterLockAcquired(false)                          //
{
    System::GiveSemaphore(resourceSemaphore);
    System::GiveSemaphore(readCountSemaphore);
    System::GiveSemaphore(serviceSemaphore);
}

bool ReaderWriterLock::AcquireReaderLock()
{
    if (!OS_RESULT_SUCCEEDED(System::TakeSemaphore(serviceSemaphore, InfiniteTimeout)))
    {
        return false;
    }

    if (!OS_RESULT_SUCCEEDED(System::TakeSemaphore(readCountSemaphore, InfiniteTimeout)))
    {
        System::GiveSemaphore(serviceSemaphore);
        return false;
    }

    if (readCount == 0)
    {
        if (!OS_RESULT_SUCCEEDED(System::TakeSemaphore(resourceSemaphore, InfiniteTimeout)))
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

bool ReaderWriterLock::AcquireWriterLock()
{
    if (!OS_RESULT_SUCCEEDED(System::TakeSemaphore(serviceSemaphore, InfiniteTimeout)))
    {
        return false;
    }

    if (!OS_RESULT_SUCCEEDED(System::TakeSemaphore(resourceSemaphore, InfiniteTimeout)))
    {
        System::GiveSemaphore(serviceSemaphore);
        return false;
    }

    isWriterLockAcquired = true;

    System::GiveSemaphore(serviceSemaphore);

    return true;
}

bool ReaderWriterLock::ReleaseReaderLock()
{
    if (readCount == 0)
    {
        return false;
    }

    if (!OS_RESULT_SUCCEEDED(System::TakeSemaphore(readCountSemaphore, InfiniteTimeout)))
    {
        return false;
    }

    readCount--;

    if (readCount == 0)
    {
        System::GiveSemaphore(resourceSemaphore);
    }

    System::GiveSemaphore(readCountSemaphore);

    return true;
}

void ReaderWriterLock::ReleaseWriterLock()
{
    if (!isWriterLockAcquired)
    {
        return;
    }

    System::GiveSemaphore(resourceSemaphore);
}

bool ReaderWriterLock::IsReaderLockAcquired() const
{
    return readCount > 0;
}

bool ReaderWriterLock::IsWriterLockAcquired() const
{
    return isWriterLockAcquired;
}

ReaderLock::ReaderLock(ReaderWriterLock& readerWriterLock) : readerWriterLock(readerWriterLock)
{
    taken = readerWriterLock.AcquireReaderLock();
}

ReaderLock::~ReaderLock()
{
    if (!taken)
    {
        return;
    }

    readerWriterLock.ReleaseReaderLock();
}

bool ReaderLock::operator()()
{
    return taken;
}

WriterLock::WriterLock(ReaderWriterLock& readerWriterLock) : readerWriterLock(readerWriterLock)
{
    taken = readerWriterLock.AcquireWriterLock();
}

WriterLock::~WriterLock()
{
    if (!taken)
    {
        return;
    }

    readerWriterLock.ReleaseWriterLock();
}

bool WriterLock::operator()()
{
    return taken;
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
