#include "I2CMock.hpp"

static I2CResult MockI2CWrite(I2CBus* bus, const I2CAddress address, uint8_t* inData, size_t length)
{
    auto mock = static_cast<I2CBusMock*>(bus);
    return mock->I2CWrite(address, inData, length);
}

static I2CResult MockI2CWriteRead(
    I2CBus* bus, const I2CAddress address, uint8_t* inData, size_t inLength, uint8_t* outData, size_t outLength)
{
    auto mock = static_cast<I2CBusMock*>(bus);
    return mock->I2CWriteRead(address, inData, inLength, outData, outLength);
}

I2CBusMock::I2CBusMock()
{
    this->Write = MockI2CWrite;
    this->WriteRead = MockI2CWriteRead;
}
