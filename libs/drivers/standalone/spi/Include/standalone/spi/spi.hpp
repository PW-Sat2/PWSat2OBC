#ifndef BOOT_SPI_HPP_
#define BOOT_SPI_HPP_

#include <cstdint>
#include <gsl/span>
#include "gpio/gpio.h"
#include "mcu/io_map.h"
#include "spi/spi.h"

class SPIPeripheral
{
  public:
    void Initialize();

    void Deinitialize();

    OSResult Write(gsl::span<const std::uint8_t> buffer);

    OSResult Read(gsl::span<std::uint8_t> buffer);

  private:
    const drivers::gpio::OutputPin<io_map::SPI::MOSI> MOSI;
    const drivers::gpio::InputPin<io_map::SPI::MISO> MISO;
    const drivers::gpio::OutputPin<io_map::SPI::CLK> CLK;
};

template <typename ChipSelect> class SPIDevice : public drivers::spi::ISPIInterface
{
  public:
    SPIDevice(SPIPeripheral& periperhal);

    void Initialize();

    virtual void Select() override;

    virtual void Deselect() override;

    virtual OSResult Write(gsl::span<const std::uint8_t> buffer) override;

    virtual OSResult Read(gsl::span<std::uint8_t> buffer) override;

  private:
    SPIPeripheral& _spi;
    drivers::gpio::OutputPin<ChipSelect> _chipSelect;
};

template <typename ChipSelect> SPIDevice<ChipSelect>::SPIDevice(SPIPeripheral& periperhal) : _spi(periperhal)
{
}

template <typename ChipSelect> void SPIDevice<ChipSelect>::Initialize()
{
    this->_chipSelect.Initialize();
}

template <typename ChipSelect> void SPIDevice<ChipSelect>::Select()
{
    this->_chipSelect.Low();
}

template <typename ChipSelect> void SPIDevice<ChipSelect>::Deselect()
{
    this->_chipSelect.High();
}

template <typename ChipSelect> OSResult SPIDevice<ChipSelect>::Write(gsl::span<const std::uint8_t> buffer)
{
    return this->_spi.Write(buffer);
}

template <typename ChipSelect> OSResult SPIDevice<ChipSelect>::Read(gsl::span<std::uint8_t> buffer)
{
    return this->_spi.Read(buffer);
}

#endif /* BOOT_SPI_HPP_ */
