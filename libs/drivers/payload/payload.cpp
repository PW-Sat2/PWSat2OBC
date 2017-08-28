#include "payload.h"
#include <em_gpio.h>
#include "gpio/InterruptPinDriver.h"
#include "logger/logger.h"

using namespace devices::payload;
using drivers::i2c::I2CResult;
using namespace std::chrono_literals;

PayloadDriver::PayloadDriver(
    error_counter::ErrorCounting& errors, drivers::i2c::II2CBus& communicationBus, drivers::gpio::IInterruptPinDriver& interruptPinDriver)
    : _error(errors), _i2c(communicationBus),  //
      _interruptPinDriver(interruptPinDriver), //
      _dataWaitTimeout(DefaultTimeout)
{
    _event.Initialize();
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
    _event.Clear(InterruptFlagFinished);
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
    auto result = _event.WaitAll(InterruptFlagFinished, true, _dataWaitTimeout);
    if (!has_flag(result, InterruptFlagFinished))
    {
        errorContext.Counter().Failure();
        LOG(LOG_LEVEL_ERROR, "Take semaphore for Payload synchronisation failed. Timeout");
        return OSResult::Timeout;
    }

    return OSResult::Success;
}

void PayloadDriver::RaiseDataReadyISR()
{
    _event.SetISR(InterruptFlagFinished);
}

void PayloadDriver::SetDataTimeout(std::chrono::milliseconds newTimeout)
{
    _dataWaitTimeout = newTimeout;
}
