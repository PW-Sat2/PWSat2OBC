#include "InterruptPinDriver.h"

#include <em_cmu.h>
#include <em_gpio.h>
#include "io_map.h"

using namespace drivers::gpio;

InterruptPinDriver::InterruptPinDriver(const Pin& pin) : _pin(pin)
{
}

uint32_t InterruptPinDriver::IRQMask()
{
    return 1 << (_pin.PinNumber());
}

void InterruptPinDriver::EnableInterrupt()
{
    auto interruptBank = _pin.PinNumber() % 2 ? GPIO_ODD_IRQn : GPIO_EVEN_IRQn;
    NVIC_SetPriority(interruptBank, io_map::Payload::InterruptPriority);
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
