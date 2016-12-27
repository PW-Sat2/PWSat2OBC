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

    GPIO_PinModeSet(SPI_PORT, SPI_MOSI, gpioModePushPull, 1);
    GPIO_PinModeSet(SPI_PORT, SPI_MISO, gpioModeInputPull, 0);
    GPIO_PinModeSet(SPI_PORT, SPI_CLK, gpioModePushPull, 1);

    DMADRV_AllocateChannel(&this->_rxChannel, nullptr);
    DMADRV_AllocateChannel(&this->_txChannel, nullptr);

    SPI_USART->ROUTE |= USART_ROUTE_CLKPEN | USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | (SPI_LOCATION << _USART_ROUTE_LOCATION_SHIFT);

    USART_Enable(SPI_USART, usartEnable);

    this->_transferGroup = System::CreateEventGroup();
    this->_lock = System::CreateBinarySemaphore();
    System::GiveSemaphore(this->_lock);
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
EFMSPISlaveInterface::EFMSPISlaveInterface(EFMSPIInterface& spi, drivers::gpio::Pin pin) : _spi(spi), _pin(pin)
{
}

void EFMSPISlaveInterface::Select()
{
    this->_spi.Lock();
    this->_pin.Low();
}

void EFMSPISlaveInterface::Deselect()
{
    this->_pin.High();
    this->_spi.Unlock();
}

void EFMSPISlaveInterface::Write(gsl::span<const std::uint8_t> buffer)
{
    this->_spi.Write(buffer);
}

void EFMSPISlaveInterface::Read(gsl::span<std::uint8_t> buffer)
{
    this->_spi.Read(buffer);
}

void drivers::spi::EFMSPIInterface::Lock()
{
    System::TakeSemaphore(this->_lock, MAX_DELAY);
}

void drivers::spi::EFMSPIInterface::Unlock()
{
    System::GiveSemaphore(this->_lock);
}
