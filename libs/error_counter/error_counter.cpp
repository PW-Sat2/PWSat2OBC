#include "error_counter.hpp"

namespace error_counter
{
    CounterValue ErrorCounting::Current(Device device) const
    {
        return static_cast<CounterValue>(this->_counters[device]);
    }

    void ErrorCounting::Failure(Device device)
    {
        auto increment = this->_config.Increment(device);

        auto prev = this->_counters[device].fetch_add(increment);

        auto overflow = (prev + increment) - std::numeric_limits<CounterValue>::max();

        if (overflow > 0)
        {
            this->_counters[device] -= overflow;
        }

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
        auto decrement = this->_config.Decrement(device);
        auto prev = this->_counters[device].fetch_sub(decrement);

        if (prev < decrement)
        {
            this->_counters[device] += (decrement - prev);
        }
    }

    ErrorCounting::ErrorCounting(IErrorCountingConfigration& config) : _counters{}, _config(config), _callback(nullptr)
    {
    }

    AggregatedErrorCounter::AggregatedErrorCounter() : _errorCount(0)
    {
    }
}
