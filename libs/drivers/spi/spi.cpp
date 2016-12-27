#include "spi.h"
#include "gpio/gpio.h"

using namespace drivers::spi;
using drivers::gpio::Pin;

SPISelectSlave::SPISelectSlave(ISPIInterface& spi) : _spi(spi)
{
    this->_spi.Select();
}

SPISelectSlave::~SPISelectSlave()
{
    this->_spi.Deselect();
}
