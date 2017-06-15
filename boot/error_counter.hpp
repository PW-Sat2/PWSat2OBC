#ifndef BOOT_ERROR_COUNTER_HPP_
#define BOOT_ERROR_COUNTER_HPP_

#include "error_counter/error_counter.hpp"

/**
 * @brief Error counter implemention for bootloader, used only to provide dependency for FRAM driver. It ignores all calls.
 */
struct BootErrorCounter : public error_counter::IErrorCounting
{
    virtual error_counter::CounterValue Current(error_counter::Device /* device*/) const override;

    virtual void Failure(error_counter::Device /*device*/) override;

    virtual void Success(error_counter::Device /*device*/) override;
};

#endif /* BOOT_ERROR_COUNTER_HPP_ */
