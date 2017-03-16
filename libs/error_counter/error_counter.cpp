#include "error_counter.hpp"

namespace error_counter
{
    CounterValue ErrorCounting::Current(Device device) const
    {
        return this->_counters[device];
    }

    void ErrorCounting::Failure(Device device)
    {
        auto increment = this->_config.Increment(device);

        auto prev = this->_counters[device].fetch_add(increment);

        if (prev + increment >= this->_config.Limit(device))
        {
            if (this->_callback != nullptr)
            {
                this->_callback->LimitReached(device, this->_counters[device]);
            }
        }
    }

    void ErrorCounting::Handler(IErrorCountingCallback& callback)
    {
        this->_callback = &callback;
    }

    void ErrorCounting::Success(Device device)
    {
        this->_counters[device] = 0;
    }

    ErrorCounting::ErrorCounting(IErrorCountingConfigration& config) : _config(config), _callback(nullptr)
    {
        for (auto& c : this->_counters)
        {
            c = 0;
        }
    }
}
