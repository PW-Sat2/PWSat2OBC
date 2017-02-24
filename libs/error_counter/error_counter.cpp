#include "error_counter.hpp"

namespace error_counter
{
    CounterValue ErrorCounting::Current(Device device)
    {
        Lock l(this->_sync, InfiniteTimeout);
        return this->_counters[device];
    }

    void ErrorCounting::Failure(Device device)
    {
        auto increment = this->_config.Increment(device);

        Lock l(this->_sync, InfiniteTimeout);

        this->_counters[device] += increment;

        if (this->_counters[device] >= 5)
        {
            if (this->_callback != nullptr)
            {
                this->_callback->LimitReached(device, this->_counters[device]);
            }
        }
    }

    void ErrorCounting::Handler(IErrorCountingCallback& callback)
    {
        Lock l(this->_sync, InfiniteTimeout);
        this->_callback = &callback;
    }

    void ErrorCounting::Success(Device device)
    {
        Lock l(this->_sync, InfiniteTimeout);
        this->_counters[device] = 0;
    }

    ErrorCounting::ErrorCounting(IErrorCountingConfigration& config) : _counters{0}, _config(config), _callback(nullptr)
    {
    }

    void ErrorCounting::Initialize()
    {
        this->_sync = System::CreateBinarySemaphore(1);
    }
}
