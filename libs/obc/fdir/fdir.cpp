#include "fdir.hpp"
#include "logger/logger.h"
#include "power/power.h"

namespace obc
{
    FDIR::FDIR(services::power::IPowerControl& powerControl) : _errorCounting(*this), _powerControl(powerControl)
    {
    }

    void FDIR::Initalize()
    {
        this->_errorCounting.Handler(*this);
    }

    error_counter::ErrorCounting& FDIR::ErrorCounting()
    {
        return this->_errorCounting;
    }

    void FDIR::LimitReached(error_counter::Device device, error_counter::CounterValue errorsCount)
    {
        LOGF(LOG_LEVEL_FATAL, "Device %d reach error limit of %d", device, errorsCount);
        this->_powerControl.PowerCycle();
    }

    error_counter::CounterValue FDIR::Limit(error_counter::Device /*device*/)
    {
        return 128;
    }

    error_counter::CounterValue FDIR::Increment(error_counter::Device /*device*/)
    {
        return 5;
    }

    error_counter::CounterValue FDIR::Decrement(error_counter::Device /*device*/)
    {
        return 2;
    }
}
