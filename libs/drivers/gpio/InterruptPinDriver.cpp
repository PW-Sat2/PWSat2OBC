#include "InterruptPinDriver.h"

#include <em_cmu.h>
#include <em_gpio.h>

using namespace drivers::gpio;

InterruptPinDriver::InterruptPinDriver(const Pin& pin) : _pin(pin)
{
}

void InterruptPinDriver::EnableInterrupt(const int32_t interruptPriority)
{
    auto interruptBank = _pin.PinNumber() % 2 ? GPIO_ODD_IRQn : GPIO_EVEN_IRQn;
    NVIC_SetPriority(interruptBank, interruptPriority);
    NVIC_EnableIRQ(interruptBank);
}
void InterruptPinDriver::ClearInterrupt()
{
    GPIO_IntClear(IRQMask());
}

bool InterruptPinDriver::Value() const
{
    return _pin.Input();
}
