#include "efm.h"

#include "efm_support/api.h"
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

static void* RXPort = const_cast<uint32_t*>(&io_map::SPI::Peripheral->RXDATA);
static void* TXPort = const_cast<uint32_t*>(&io_map::SPI::Peripheral->TXDATA);

void EFMSPIInterface::Initialize()
{
    efm::cmu::ClockEnable(efm::Clock(io_map::SPI::Peripheral), true);

    USART_InitSync_TypeDef init = USART_INITSYNC_DEFAULT;
    init.master = true;
    init.baudrate = 7_MHz;
    init.msbf = true;
    init.clockMode = usartClockMode0;
    init.databits = usartDatabits8;
    init.autoTx = false;
    init.enable = usartDisable;

    efm::usart::InitSync(io_map::SPI::Peripheral, &init);

    efm::dma::AllocateChannel(&this->_rxChannel, nullptr);
    efm::dma::AllocateChannel(&this->_txChannel, nullptr);

    efm::usart::AmendRoute(io_map::SPI::Peripheral,
        USART_ROUTE_CLKPEN | USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | (io_map::SPI::Location << _USART_ROUTE_LOCATION_SHIFT));

    efm::usart::Enable(io_map::SPI::Peripheral, usartEnable);

    this->_transferGroup = System::CreateEventGroup();
    this->_lock = System::CreateBinarySemaphore();
    System::GiveSemaphore(this->_lock);
}

void EFMSPIInterface::Write(gsl::span<const std::uint8_t> buffer)
{
    System::EventGroupClearBits(this->_transferGroup, TransferFinished);

    efm::usart::Command(io_map::SPI::Peripheral, USART_CMD_CLEARRX | USART_CMD_CLEARTX);

    efm::usart::IntClear(io_map::SPI::Peripheral, efm::usart::IntGet(io_map::SPI::Peripheral));

    uint32_t dummyRx = 0;
    efm::dma::PeripheralMemory(this->_rxChannel,
        efm::DMASignal<efm::DMASignalUSART::RXDATAV>(io_map::SPI::Peripheral),
        &dummyRx,
        RXPort,
        false,
        buffer.size(),
        dmadrvDataSize1,
        OnTransferFinished,
        this);

    efm::dma::MemoryPeripheral(this->_txChannel,
        efm::DMASignal<efm::DMASignalUSART::TXBL>(io_map::SPI::Peripheral),
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

    efm::usart::Command(io_map::SPI::Peripheral, USART_CMD_CLEARRX | USART_CMD_CLEARTX);

    efm::usart::IntClear(io_map::SPI::Peripheral, efm::usart::IntGet(io_map::SPI::Peripheral));

    efm::dma::PeripheralMemory(this->_rxChannel,
        efm::DMASignal<efm::DMASignalUSART::RXDATAV>(io_map::SPI::Peripheral),
        buffer.data(),
        RXPort,
        true,
        buffer.size(),
        dmadrvDataSize1,
        OnTransferFinished,
        this);

    uint32_t dummyTx = 0;
    efm::dma::MemoryPeripheral(this->_txChannel,
        efm::DMASignal<efm::DMASignalUSART::TXBL>(io_map::SPI::Peripheral),
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
EFMSPISlaveInterface::EFMSPISlaveInterface(EFMSPIInterface& spi, const drivers::gpio::Pin& pin) : _spi(spi), _pin(pin)
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
