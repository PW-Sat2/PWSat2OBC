#include "efm.h"
#include <em_gpio.h>
#include <em_usart.h>

#include "system.h"

using gsl::span;

using namespace drivers::spi;

void EFMSPIInterface::Select()
{
    GPIO_PinOutClear(gpioPortD, 3);
}

void EFMSPIInterface::Write(gsl::span<const std::uint8_t> buffer)
{
    UNREFERENCED_PARAMETER(buffer);
}

void EFMSPIInterface::Read(gsl::span<std::uint8_t> buffer)
{
    UNREFERENCED_PARAMETER(buffer);
}

void EFMSPIInterface::Deselect()
{
    GPIO_PinOutSet(gpioPortD, 3);
}

void EFMSPIInterface::Initialize()
{
    GPIO_PinModeSet(gpioPortD, 3, gpioModePushPull, 1); // cs
}

void EFMSPIInterface::WriteRead(gsl::span<const std::uint8_t> input, gsl::span<std::uint8_t> output)
{
    this->Write(input);
    this->Read(output);
}
