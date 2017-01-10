#include <array>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "efm_support/clock.h"
#include "efm_support/dma.h"
#include "io_map.h"
#include "mock/emlib.h"
#include "spi/efm.h"

using testing::_;
using testing::Test;
using testing::NiceMock;
using testing::DoAll;
using testing::SetArgPointee;
using testing::Return;
using testing::InSequence;

using drivers::spi::EFMSPIInterface;

class SPIDriverTest : public Test
{
  public:
    SPIDriverTest();

  protected:
    NiceMock<CMUMock> _cmu;
    CMUReset _cmuReset;

    NiceMock<DMAMock> _dma;
    DMAReset _dmaReset;

    NiceMock<USARTMock> _usart;
    USARTReset _usartReset;
};

SPIDriverTest::SPIDriverTest()
{
    this->_cmuReset = InstallProxy(&this->_cmu);
    this->_dmaReset = InstallProxy(&this->_dma);
    this->_usartReset = InstallProxy(&this->_usart);
}

TEST_F(SPIDriverTest, ShouldInitializeProperly)
{
    EXPECT_CALL(this->_cmu, ClockEnable(efm::Clock(io_map::SPI::Peripheral), true));
    EXPECT_CALL(this->_usart, InitSync(io_map::SPI::Peripheral, _));
    EXPECT_CALL(this->_usart,
        AmendRoute(io_map::SPI::Peripheral,
            USART_ROUTE_CLKPEN | USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | (io_map::SPI::Location << _USART_ROUTE_LOCATION_SHIFT)));
    EXPECT_CALL(this->_usart, Enable(io_map::SPI::Peripheral, usartEnable));

    EFMSPIInterface spi;

    spi.Initialize();
}

TEST_F(SPIDriverTest, ShouldReadProperly)
{
    auto rxSignal = efm::DMASignal<efm::DMASignalUSART::RXDATAV>(io_map::SPI::Peripheral);
    auto txSignal = efm::DMASignal<efm::DMASignalUSART::TXBL>(io_map::SPI::Peripheral);

    {
        InSequence s;

        EXPECT_CALL(this->_dma, AllocateChannel(_, nullptr)).WillOnce(DoAll(SetArgPointee<0>(1), Return(ECODE_EMDRV_DMADRV_OK)));
        EXPECT_CALL(this->_dma, AllocateChannel(_, nullptr)).WillOnce(DoAll(SetArgPointee<0>(2), Return(ECODE_EMDRV_DMADRV_OK)));

        EXPECT_CALL(this->_usart, Command(io_map::SPI::Peripheral, USART_CMD_CLEARRX | USART_CMD_CLEARTX));
        EXPECT_CALL(this->_usart, IntClear(io_map::SPI::Peripheral, _));

        EXPECT_CALL(this->_dma, PeripheralMemory(1, rxSignal, _, _, true, 10, dmadrvDataSize1, _, _));
        EXPECT_CALL(this->_dma, MemoryPeripheral(2, txSignal, _, _, false, 10, dmadrvDataSize1, _, _));
    }

    EFMSPIInterface spi;

    spi.Initialize();

    std::array<uint8_t, 10> buf;

    spi.Read(buf);
}

TEST_F(SPIDriverTest, ShouldWriteProperly)
{
    auto rxSignal = efm::DMASignal<efm::DMASignalUSART::RXDATAV>(io_map::SPI::Peripheral);
    auto txSignal = efm::DMASignal<efm::DMASignalUSART::TXBL>(io_map::SPI::Peripheral);

    {
        InSequence s;

        EXPECT_CALL(this->_dma, AllocateChannel(_, nullptr)).WillOnce(DoAll(SetArgPointee<0>(1), Return(ECODE_EMDRV_DMADRV_OK)));
        EXPECT_CALL(this->_dma, AllocateChannel(_, nullptr)).WillOnce(DoAll(SetArgPointee<0>(2), Return(ECODE_EMDRV_DMADRV_OK)));

        EXPECT_CALL(this->_usart, Command(io_map::SPI::Peripheral, USART_CMD_CLEARRX | USART_CMD_CLEARTX));
        EXPECT_CALL(this->_usart, IntClear(io_map::SPI::Peripheral, _));

        EXPECT_CALL(this->_dma, PeripheralMemory(1, rxSignal, _, _, false, 10, dmadrvDataSize1, _, _));
        EXPECT_CALL(this->_dma, MemoryPeripheral(2, txSignal, _, _, true, 10, dmadrvDataSize1, _, _));
    }

    EFMSPIInterface spi;

    spi.Initialize();

    std::array<uint8_t, 10> buf;

    spi.Write(buf);
}
