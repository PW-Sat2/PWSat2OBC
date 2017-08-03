#include "payload.h"
#include <em_gpio.h>
#include "gpio/InterruptPinDriver.h"
#include "logger/logger.h"

using namespace drivers::payload;
using drivers::i2c::I2CResult;

PayloadDriver::PayloadDriver(
    error_counter::ErrorCounting& errors, drivers::i2c::II2CBus& communicationBus, drivers::gpio::IInterruptPinDriver& interruptPinDriver)
    : _error(errors), _i2c(communicationBus),  //
      _interruptPinDriver(interruptPinDriver), //
      _sync(nullptr),                          //
      _dataWaitTimeout(DefaultTimeout)
{
}

void PayloadDriver::IRQHandler()
{
    auto value = _interruptPinDriver.Value();
    if (!value)
    {
        RaiseDataReadyISR();
    }
}

void PayloadDriver::Initialize()
{
    _sync = System::CreateBinarySemaphore();
    _interruptPinDriver.EnableInterrupt();
}

bool PayloadDriver::IsBusy() const
{
    return _interruptPinDriver.Value();
}

OSResult PayloadDriver::PayloadRead(gsl::span<std::uint8_t> outData, gsl::span<std::uint8_t> inData)
{
    ErrorReporter errorContext(_error);
    auto result = _i2c.WriteRead(I2CAddress, outData, inData);
    const bool status = (result == I2CResult::OK);
    if (!status)
    {
        errorContext.Counter().Failure();
        LOGF(LOG_LEVEL_ERROR, "Unable to perform Payload I2C Write and Read. Reason: %d", num(result));
        return OSResult::InvalidOperation;
    }

    return OSResult::Success;
}

OSResult PayloadDriver::PayloadWrite(gsl::span<std::uint8_t> outData)
{
    ErrorReporter errorContext(_error);
    auto result = _i2c.Write(I2CAddress, outData);
    const bool status = (result == I2CResult::OK);
    if (!status)
    {
        errorContext.Counter().Failure();
        LOGF(LOG_LEVEL_ERROR, "Unable to perform Payload I2C Write. Reason: %d", num(result));
        return OSResult::InvalidOperation;
    }

    return OSResult::Success;
}

OSResult PayloadDriver::WaitForData()
{
    ErrorReporter errorContext(_error);
    auto result = System::TakeSemaphore(_sync, _dataWaitTimeout);
    if (result != OSResult::Success)
    {
        errorContext.Counter().Failure();
        LOGF(LOG_LEVEL_ERROR, "Take semaphore for Payload synchronisation failed. Reason: %d", num(result));
        return result;
    }

    return OSResult::Success;
}

void PayloadDriver::RaiseDataReadyISR()
{
    System::GiveSemaphoreISR(_sync);
}

void PayloadDriver::SetDataTimeout(std::chrono::milliseconds newTimeout)
{
    _dataWaitTimeout = newTimeout;
}
