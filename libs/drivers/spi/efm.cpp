#include "efm.h"
#include <em_gpio.h>

using gsl::span;

using namespace drivers::spi;

void EFMSPIInterface::Select()
{
    GPIO_PinOutClear(gpioPortD, 3);
}

void EFMSPIInterface::Write(gsl::span<const std::uint8_t> buffer)
{
    SPIDRV_MTransmitB(&this->_handle, buffer.data(), buffer.size());
}

void EFMSPIInterface::Read(gsl::span<std::uint8_t> buffer)
{
    SPIDRV_MReceiveB(&this->_handle, buffer.data(), buffer.size());
}

void EFMSPIInterface::Deselect()
{
    GPIO_PinOutSet(gpioPortD, 3);
}

void EFMSPIInterface::Initialize()
{
    GPIO_PinModeSet(gpioPortD, 3, gpioModePushPull, 1); // cs

    SPIDRV_Init_t init;
    init.bitOrder = SPIDRV_BitOrder::spidrvBitOrderMsbFirst;
    init.bitRate = 8000000;
    init.clockMode = SPIDRV_ClockMode::spidrvClockMode0;
    init.csControl = SPIDRV_CsControl::spidrvCsControlApplication;
    init.dummyTxValue = 0;
    init.frameLength = 8;
    init.port = USART1;
    init.portLocation = 1;
    init.type = SPIDRV_Type::spidrvMaster;

    SPIDRV_Init(&this->_handle, &init);
}

void EFMSPIInterface::WriteRead(gsl::span<const std::uint8_t> input, gsl::span<std::uint8_t> output)
{
    this->Write(input);
    this->Read(output);
}
