#include "spi.hpp"
#include <em_cmu.h>
#include <em_usart.h>

void SPIPeripheral::Initialize()
{
    CMU_ClockEnable(cmuClock_USART0, true);
    USART_InitSync_TypeDef init = USART_INITSYNC_DEFAULT;
    init.master = true;
    init.baudrate = 1;
    init.msbf = true;
    init.clockMode = usartClockMode0;
    init.databits = usartDatabits8;
    init.autoTx = false;
    init.enable = usartDisable;
    USART_InitSync(io_map::SPI::Peripheral, &init);
    io_map::SPI::Peripheral->ROUTE |=
        USART_ROUTE_CLKPEN | USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | (io_map::SPI::Location << _USART_ROUTE_LOCATION_SHIFT);
    MOSI.Initialize();
    MISO.Initialize();
    CLK.Initialize();
    USART_Enable(io_map::SPI::Peripheral, usartEnable);
}

void SPIPeripheral::Deinitialize()
{
    USART_Enable(io_map::SPI::Peripheral, usartDisable);
}

void SPIPeripheral::Write(gsl::span<const std::uint8_t> buffer)
{
    for (const auto b : buffer)
    {
        USART_SpiTransfer(io_map::SPI::Peripheral, b);
    }
}

void SPIPeripheral::Read(gsl::span<std::uint8_t> buffer)
{
    for (auto& b : buffer)
    {
        b = USART_SpiTransfer(io_map::SPI::Peripheral, 0);
    }
}
