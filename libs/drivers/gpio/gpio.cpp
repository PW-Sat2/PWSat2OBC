#include "gpio.h"

using namespace drivers::gpio;

Pin::Pin(GPIO_Port_TypeDef port, std::uint16_t pin) : _port(port), _pin(pin)
{
}
