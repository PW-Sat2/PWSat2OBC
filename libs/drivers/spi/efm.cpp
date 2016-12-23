#include "efm.h"
#include <em_cmu.h>
#include <em_gpio.h>
#include <em_int.h>
#include <em_usart.h>
#include <core_cm3.h>

#include <dmadrv.h>
#include "efm_support/clock.h"
#include "efm_support/dma.h"
#include "io_map.h"
#include "logger/logger.h"
#include "system.h"
#include "utils.h"

using gsl::span;
using std::uint8_t;
using std::uint32_t;

using namespace drivers::spi;

static void* RXPort = const_cast<uint32_t*>(&SPI_USART->RXDATA);
static void* TXPort = const_cast<uint32_t*>(&SPI_USART->TXDATA);

void EFMSPIInterface::Initialize()
{
    CMU_ClockEnable(efm::Clock(SPI_USART), true);

    USART_InitSync_TypeDef init = USART_INITSYNC_DEFAULT;
    init.master = true;
    init.baudrate = 7_MHz;
    init.msbf = true;
    init.clockMode = usartClockMode0;
    init.databits = usartDatabits8;
    init.autoTx = false;
    init.enable = usartDisable;

    USART_InitSync(SPI_USART, &init);

    GPIO_PinModeSet(gpioPortD, 3, gpioModePushPull, 1); // cs

    GPIO_PinModeSet(SPI_PORT, SPI_MOSI, gpioModePushPull, 1);
    GPIO_PinModeSet(SPI_PORT, SPI_MISO, gpioModeInputPull, 0);
    GPIO_PinModeSet(SPI_PORT, SPI_CLK, gpioModePushPull, 1);

    DMADRV_AllocateChannel(&this->_rxChannel, nullptr);
    DMADRV_AllocateChannel(&this->_txChannel, nullptr);

    this->_transferGroup = System::CreateEventGroup();

    SPI_USART->ROUTE |= USART_ROUTE_CLKPEN | USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | (SPI_LOCATION << _USART_ROUTE_LOCATION_SHIFT);

    USART_Enable(SPI_USART, usartEnable);
}

void EFMSPIInterface::Select()
{
    GPIO_PinOutClear(gpioPortD, 3);
}

void EFMSPIInterface::Write(gsl::span<const std::uint8_t> buffer)
{
    System::EventGroupClearBits(this->_transferGroup, TransferFinished);

    SPI_USART->CMD = USART_CMD_CLEARRX | USART_CMD_CLEARTX;

    USART_IntClear(SPI_USART, USART_IntGet(SPI_USART));

    uint32_t dummyRx = 0;
    DMADRV_PeripheralMemory(this->_rxChannel,
        efm::DMASignal<efm::DMASignalUSART::RXDATAV>(SPI_USART),
        &dummyRx,
        RXPort,
        false,
        buffer.size(),
        dmadrvDataSize1,
        OnTransferFinished,
        this);

    DMADRV_MemoryPeripheral(this->_txChannel,
        efm::DMASignal<efm::DMASignalUSART::TXBL>(SPI_USART),
        TXPort,
        const_cast<uint8_t*>(buffer.data()),
        true,
        buffer.size(),
        dmadrvDataSize1,
        OnTransferFinished,
        this);

    System::EventGroupWaitForBits(this->_transferGroup, TransferFinished, true, true, MAX_DELAY);
}

void EFMSPIInterface::Read(gsl::span<std::uint8_t> buffer)
{
    System::EventGroupClearBits(this->_transferGroup, TransferFinished);

    SPI_USART->CMD = USART_CMD_CLEARRX | USART_CMD_CLEARTX;

    USART_IntClear(SPI_USART, USART_IntGet(SPI_USART));

    DMADRV_PeripheralMemory(this->_rxChannel,
        efm::DMASignal<efm::DMASignalUSART::RXDATAV>(SPI_USART),
        buffer.data(),
        RXPort,
        true,
        buffer.size(),
        dmadrvDataSize1,
        OnTransferFinished,
        this);

    uint32_t dummyTx = 0;
    DMADRV_MemoryPeripheral(this->_txChannel,
        efm::DMASignal<efm::DMASignalUSART::TXBL>(SPI_USART),
        TXPort,
        &dummyTx,
        false,
        buffer.size(),
        dmadrvDataSize1,
        OnTransferFinished,
        this);

    System::EventGroupWaitForBits(this->_transferGroup, TransferFinished, true, true, MAX_DELAY);
}

void EFMSPIInterface::Deselect()
{
    GPIO_PinOutSet(gpioPortD, 3);
}

void EFMSPIInterface::WriteRead(gsl::span<const std::uint8_t> input, gsl::span<std::uint8_t> output)
{
    this->Write(input);
    this->Read(output);
}

bool EFMSPIInterface::OnTransferFinished(unsigned int channel, unsigned int sequenceNo, void* param)
{
    UNUSED(channel, sequenceNo);

    auto This = static_cast<EFMSPIInterface*>(param);

    if (channel == This->_rxChannel)
    {
        System::EventGroupSetBitsISR(This->_transferGroup, TransferRXFinished);
    }
    else if (channel == This->_txChannel)
    {
        System::EventGroupSetBitsISR(This->_transferGroup, TransferTXFinished);
    }

    System::EndSwitchingISR();

    return true;
}
