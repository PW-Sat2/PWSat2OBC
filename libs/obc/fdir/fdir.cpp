#include "fdir.hpp"
#include "logger/logger.h"

namespace obc
{
    FDIR::FDIR() : _errorCounting(*this)
    {
    }

    void FDIR::InitalizeRunlevel1()
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
    }

    error_counter::CounterValue FDIR::Limit(error_counter::Device /*device*/)
    {
        return 5;
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
