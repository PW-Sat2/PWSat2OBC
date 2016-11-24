#include "spi.h"

using namespace drivers::spi;

SPISelectSlave::SPISelectSlave(ISPIInterface& spi) : _spi(spi)
{
    this->_spi.Select();
}

SPISelectSlave::~SPISelectSlave()
{
    this->_spi.Deselect();
}
