
#include "error_counter.hpp"

error_counter::CounterValue BootErrorCounter::Current(error_counter::Device /* device*/) const
{
    return 0;
}

void BootErrorCounter::Failure(error_counter::Device /*device*/)
{
}

void BootErrorCounter::Success(error_counter::Device /*device*/)
{
}
