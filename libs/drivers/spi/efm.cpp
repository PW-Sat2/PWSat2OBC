#include "efm.h"
#include <em_cmu.h>
#include <em_gpio.h>
#include <em_int.h>
#include <em_usart.h>
#include <core_cm3.h>

#include <dmadrv.h>
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
    CMU_ClockEnable(cmuClock_USART1, true);

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
    this->_lock = System::CreateBinarySemaphore();
    System::GiveSemaphore(this->_lock);

    SPI_USART->ROUTE |= USART_ROUTE_CLKPEN | USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | (SPI_LOCATION << _USART_ROUTE_LOCATION_SHIFT);

    USART_Enable(SPI_USART, usartEnable);

    //    LOGF(LOG_LEVEL_INFO, "DMA RX %d TX %d", this->_rxChannel, this->_txChannel);
}

void EFMSPIInterface::Select()
{
    GPIO_PinOutClear(gpioPortD, 3);
}

void EFMSPIInterface::Write(gsl::span<const std::uint8_t> buffer)
{
    Lock lock(this->_lock, MAX_DELAY);

    //    LOGF(LOG_LEVEL_INFO, "Write %d", buffer.size());

    System::EventGroupClearBits(this->_transferGroup, TransferFinished);

    uint32_t dummyRx = 0;
    SPI_USART->CMD = USART_CMD_CLEARRX | USART_CMD_CLEARTX;
    USART_IntClear(SPI_USART, USART_IntGet(SPI_USART));

    DMADRV_PeripheralMemory(this->_rxChannel,
        dmadrvPeripheralSignal_USART1_RXDATAV,
        &dummyRx,
        RXPort,
        false,
        buffer.size(),
        dmadrvDataSize1,
        OnTransferFinished,
        this);

    DMADRV_MemoryPeripheral(this->_txChannel,
        dmadrvPeripheralSignal_USART1_TXBL,
        TXPort,
        const_cast<uint8_t*>(buffer.data()),
        true,
        buffer.size(),
        dmadrvDataSize1,
        OnTransferFinished,
        this);

    System::EventGroupWaitForBits(this->_transferGroup, TransferFinished, true, true, MAX_DELAY);

    //    PrintDMAInfo();
}

void EFMSPIInterface::Read(gsl::span<std::uint8_t> buffer)
{
    Lock lock(this->_lock, MAX_DELAY);

    //    LOGF(LOG_LEVEL_INFO, "Read %d", buffer.size());

    System::EventGroupClearBits(this->_transferGroup, TransferFinished);

    SPI_USART->CMD = USART_CMD_CLEARRX | USART_CMD_CLEARTX;
    USART_IntClear(SPI_USART, USART_IntGet(SPI_USART));

    auto e1 = DMADRV_PeripheralMemory(this->_rxChannel,
        dmadrvPeripheralSignal_USART1_RXDATAV,
        buffer.data(),
        RXPort, // const_cast<uint32_t*>(&SPI_USART->RXDATA),
        true,
        buffer.size(),
        dmadrvDataSize1,
        OnTransferFinished,
        this);

    uint32_t dummyTx = 0;

    auto e2 = DMADRV_MemoryPeripheral(this->_txChannel,
        dmadrvPeripheralSignal_USART1_TXBL,
        TXPort,
        &dummyTx,
        false,
        buffer.size(),
        dmadrvDataSize1,
        OnTransferFinished,
        this);

    UNUSED(e1, e2);

    //    if (e1 != ECODE_OK || e2 != ECODE_OK)
    {
        //        LOGF(LOG_LEVEL_FATAL, "E1=%lX E2=%lX ISER=%lX%lX%lX", e1, e2, NVIC->ISER[2], NVIC->ISER[1], NVIC->ISER[0]);
    }

    System::EventGroupWaitForBits(this->_transferGroup, TransferFinished, true, true, MAX_DELAY);

    //    PrintDMAInfo();
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
    INT_Disable();
    UNUSED(channel, sequenceNo);
    auto This = static_cast<EFMSPIInterface*>(param);
    //    LOGF_ISR(LOG_LEVEL_DEBUG, "DMA complete %s", channel == This->_rxChannel ? "RX" : "TX");

    if (channel == This->_rxChannel)
    {
        System::EventGroupSetBitsISR(This->_transferGroup, TransferRXFinished);
    }
    else if (channel == This->_txChannel)
    {
        System::EventGroupSetBitsISR(This->_transferGroup, TransferTXFinished);
    }

    System::EndSwitchingISR();

    INT_Enable();

    return true;
}

void drivers::spi::EFMSPIInterface::PrintDMAInfo()
{
    int rxRem = 10, txRem = 10;

    DMADRV_TransferRemainingCount(this->_rxChannel, &rxRem);
    DMADRV_TransferRemainingCount(this->_txChannel, &txRem);

    LOGF(LOG_LEVEL_INFO, "RX (rem %d) TX (rem %d)", rxRem, txRem);
}
