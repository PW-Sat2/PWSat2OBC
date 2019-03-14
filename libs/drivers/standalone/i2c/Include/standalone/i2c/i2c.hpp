#pragma once
#include <em_cmu.h>
#include <em_gpio.h>
#include <em_i2c.h>
#include "i2c/i2c.h"

class StandaloneI2C : drivers::i2c::II2CBus
{
  public:
    StandaloneI2C(I2C_TypeDef* hw);

    void Initialize(CMU_Clock_TypeDef clock,
        GPIO_Port_TypeDef sdaPort,
        std::uint8_t sdaPin,
        GPIO_Port_TypeDef sclPort,
        std::uint8_t sclPin,
        std::uint32_t location);

    virtual drivers::i2c::I2CResult Write(const drivers::i2c::I2CAddress address, gsl::span<const uint8_t> inData) override;
    virtual drivers::i2c::I2CResult Read(const drivers::i2c::I2CAddress address, gsl::span<uint8_t> outData) override;
    virtual drivers::i2c::I2CResult WriteRead(
        const drivers::i2c::I2CAddress address, gsl::span<const uint8_t> inData, gsl::span<uint8_t> outData) override;

  private:
    drivers::i2c::I2CResult ExecuteTransfer(I2C_TransferSeq_TypeDef* seq);

    I2C_TypeDef* _hw;
};