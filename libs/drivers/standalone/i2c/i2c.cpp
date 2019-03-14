#include "i2c.hpp"

StandaloneI2C::StandaloneI2C(I2C_TypeDef* hw) : _hw(hw)
{
}

void StandaloneI2C::Initialize(CMU_Clock_TypeDef clock,
    GPIO_Port_TypeDef sdaPort,
    std::uint8_t sdaPin,
    GPIO_Port_TypeDef sclPort,
    std::uint8_t sclPin,
    std::uint32_t location)
{
    CMU_ClockEnable(clock, true);

    GPIO_PinModeSet(sdaPort, sdaPin, gpioModeWiredAndPullUpFilter, 1);
    GPIO_PinModeSet(sclPort, sclPin, gpioModeWiredAndPullUpFilter, 1);

    I2C_Init_TypeDef init = I2C_INIT_DEFAULT;
    init.clhr = i2cClockHLRStandard;
    init.enable = true;

    I2C_Init(_hw, &init);
    _hw->ROUTE = I2C_ROUTE_SCLPEN | I2C_ROUTE_SDAPEN | location;
}

drivers::i2c::I2CResult StandaloneI2C::Write(const drivers::i2c::I2CAddress address, gsl::span<const uint8_t> inData)
{
    I2C_TransferSeq_TypeDef seq;
    seq.addr = (address << 1);
    seq.flags = I2C_FLAG_WRITE;
    seq.buf[0].len = inData.length();
    seq.buf[0].data = const_cast<uint8_t*>(inData.data());
    seq.buf[1].len = 0;
    seq.buf[1].data = nullptr;

    return ExecuteTransfer(&seq);
}
drivers::i2c::I2CResult StandaloneI2C::Read(const drivers::i2c::I2CAddress address, gsl::span<uint8_t> outData)
{
    I2C_TransferSeq_TypeDef seq;
    seq.addr = (address << 1);
    seq.flags = I2C_FLAG_READ;
    seq.buf[0].len = outData.length();
    seq.buf[0].data = const_cast<uint8_t*>(outData.data());
    seq.buf[1].len = 0;
    seq.buf[1].data = nullptr;

    return ExecuteTransfer(&seq);
}
drivers::i2c::I2CResult StandaloneI2C::WriteRead(
    const drivers::i2c::I2CAddress address, gsl::span<const uint8_t> inData, gsl::span<uint8_t> outData)
{
    I2C_TransferSeq_TypeDef seq;
    seq.addr = (address << 1);
    seq.flags = I2C_FLAG_WRITE_READ;
    seq.buf[0].len = inData.length();
    seq.buf[0].data = const_cast<uint8_t*>(inData.data());
    seq.buf[1].len = outData.length();
    seq.buf[1].data = outData.data();

    return ExecuteTransfer(&seq);
}

drivers::i2c::I2CResult StandaloneI2C::ExecuteTransfer(I2C_TransferSeq_TypeDef* seq)
{
    auto rawResult = I2C_TransferInit(this->_hw, seq);

    while (rawResult == i2cTransferInProgress)
    {
        rawResult = I2C_Transfer(this->_hw);
    }

    return static_cast<drivers::i2c::I2CResult>(rawResult);
}
