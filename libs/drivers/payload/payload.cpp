#include "payload.h"
#include <em_gpio.h>
#include "logger/logger.h"
#include "telemetry.h"

using namespace drivers::payload;
using drivers::i2c::I2CResult;

PayloadDriver::PayloadDriver(drivers::i2c::II2CBus& communicationBus, const drivers::gpio::Pin& interruptPin)
    : _i2c(communicationBus),      //
      _interruptPin(interruptPin), //
      _sync(nullptr)               //
{
}

void PayloadDriver::IRQHandler()
{
    GPIO_IntClear(IRQMask());
    System::GiveSemaphoreISR(_sync);
    System::EndSwitchingISR();
}

void PayloadDriver::Initialize()
{
    _sync = System::CreateBinarySemaphore();

    auto interruptBank = _interruptPin.PinNumber() % 2 ? GPIO_ODD_IRQn : GPIO_EVEN_IRQn;
    NVIC_EnableIRQ(interruptBank);
}

OSResult PayloadDriver::RefreshRadFET()
{
    OSResult result = PerformCommand(PayloadCommands::RefreshRadFET);
    if (result != OSResult::Success)
    {
        return result;
    }

    System::TakeSemaphore(_sync, std::chrono::milliseconds(DefaultTimeout2));

    size_t address = offsetof(PayloadTelemetry, radfet);

    union {
        std::array<uint8_t, sizeof(PayloadTelemetry::Radfet)> buffer;
        PayloadTelemetry::Radfet radFETTelemetry;
    } telemetry;

    result = PerformDataRead(address, telemetry.buffer);
    if (result != OSResult::Success)
    {
        return result;
    }

    return OSResult::Success;
}

OSResult PayloadDriver::PerformCommand(PayloadCommands command)
{
    if (_interruptPin.Input())
    {
        LOG(LOG_LEVEL_WARNING, "Payload busy. Ignoring command");
        return OSResult::Busy;
    }

    std::array<std::uint8_t, 1> commandBuffer;
    commandBuffer[0] = num(command);

    const I2CResult result = _i2c.Write(I2CAddress, commandBuffer);
    const bool status = (result == I2CResult::OK);
    if (!status)
    {
        LOGF(LOG_LEVEL_ERROR, "Unable to perform command %d. Reason: %d", num(command), num(result));
        return OSResult::InvalidOperation;
    }

    return OSResult::Success;
}

OSResult PayloadDriver::PerformDataRead(uint8_t address, gsl::span<uint8_t> buffer)
{
    std::array<std::uint8_t, 1> commandBuffer = {address};

    const I2CResult result = _i2c.WriteRead(I2CAddress, commandBuffer, buffer);
    const bool status = (result == I2CResult::OK);
    if (!status)
    {
        LOGF(LOG_LEVEL_ERROR, "Unable to perform RadFET refresh. Reason: %d", num(result));
        return OSResult::InvalidOperation;
    }

    return OSResult::Success;
}
