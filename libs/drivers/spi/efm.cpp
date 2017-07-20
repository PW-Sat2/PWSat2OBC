#include "efm.h"

#include <em_gpio.h>
#include "efm_support/api.h"
#include "efm_support/clock.h"
#include "efm_support/dma.h"
#include "logger/logger.h"
#include "mcu/io_map.h"
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
    init.baudrate = io_map::SPI::Baudrate;
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

    this->_transferGroup.Initialize();
    this->_lock = System::CreateBinarySemaphore();
    System::GiveSemaphore(this->_lock);
}

OSResult EFMSPIInterface::Write(gsl::span<const std::uint8_t> buffer)
{
    efm::usart::Command(io_map::SPI::Peripheral, USART_CMD_RXBLOCKEN);

    for (decltype(buffer.size()) offset = 0; offset < buffer.size(); offset += 1024)
    {
        auto part = buffer.subspan(offset, std::min(1024, buffer.size() - offset));

        this->_transferGroup.Clear(TransferFinished);

        efm::usart::Command(io_map::SPI::Peripheral, USART_CMD_CLEARRX);
        efm::usart::Command(io_map::SPI::Peripheral, USART_CMD_CLEARTX);

        efm::usart::IntClear(io_map::SPI::Peripheral, efm::usart::IntGet(io_map::SPI::Peripheral));

        efm::dma::MemoryPeripheral(this->_txChannel,
            efm::DMASignal<efm::DMASignalUSART::TXBL>(io_map::SPI::Peripheral),
            TXPort,
            const_cast<uint8_t*>(part.data()),
            true,
            part.size(),
            dmadrvDataSize1,
            OnTransferFinished,
            this);

        auto result = this->_transferGroup.WaitAll(TransferTXFinished, true, io_map::SPI::DMATransferTimeout);

        if (!has_flag(result, TransferTXFinished))
        {
            efm::usart::Command(io_map::SPI::Peripheral, USART_CMD_RXBLOCKDIS);
            return OSResult::Timeout;
        }

        efm::usart::IntClear(io_map::SPI::Peripheral, efm::usart::IntGet(io_map::SPI::Peripheral));
    }

    efm::usart::Command(io_map::SPI::Peripheral, USART_CMD_RXBLOCKDIS);

    return OSResult::Success;
}

OSResult EFMSPIInterface::Read(gsl::span<std::uint8_t> buffer)
{
    for (decltype(buffer.size()) offset = 0; offset < buffer.size(); offset += 1024)
    {
        auto part = buffer.subspan(offset, std::min(1024, buffer.size() - offset));

        this->_transferGroup.Clear(TransferFinished);

        efm::usart::Command(io_map::SPI::Peripheral, USART_CMD_CLEARRX);
        efm::usart::Command(io_map::SPI::Peripheral, USART_CMD_CLEARTX);

        efm::usart::IntClear(io_map::SPI::Peripheral, efm::usart::IntGet(io_map::SPI::Peripheral));

        efm::dma::PeripheralMemory(this->_rxChannel,
            efm::DMASignal<efm::DMASignalUSART::RXDATAV>(io_map::SPI::Peripheral),
            part.data(),
            RXPort,
            true,
            part.size(),
            dmadrvDataSize1,
            OnTransferFinished,
            this);

        efm::dma::MemoryPeripheral(this->_txChannel,
            efm::DMASignal<efm::DMASignalUSART::TXBL>(io_map::SPI::Peripheral),
            TXPort,
            part.data(),
            true,
            part.size(),
            dmadrvDataSize1,
            OnTransferFinished,
            this);

        auto result = this->_transferGroup.WaitAll(TransferFinished, true, io_map::SPI::DMATransferTimeout);

        if (!has_flag(result, TransferFinished))
        {
            return OSResult::Timeout;
        }
    }

    return OSResult::Success;
}

bool EFMSPIInterface::OnTransferFinished(unsigned int channel, unsigned int sequenceNo, void* param)
{
    UNUSED(channel, sequenceNo);

    auto This = static_cast<EFMSPIInterface*>(param);

    if (channel == This->_rxChannel)
    {
        This->_transferGroup.SetISR(TransferRXFinished);
    }
    else if (channel == This->_txChannel)
    {
        This->_transferGroup.SetISR(TransferTXFinished);
    }

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

OSResult EFMSPISlaveInterface::Write(gsl::span<const std::uint8_t> buffer)
{
    return this->_spi.Write(buffer);
}

OSResult EFMSPISlaveInterface::Read(gsl::span<std::uint8_t> buffer)
{
    return this->_spi.Read(buffer);
}

void drivers::spi::EFMSPIInterface::Lock()
{
    System::TakeSemaphore(this->_lock, InfiniteTimeout);
}

void drivers::spi::EFMSPIInterface::Unlock()
{
    System::GiveSemaphore(this->_lock);
}
