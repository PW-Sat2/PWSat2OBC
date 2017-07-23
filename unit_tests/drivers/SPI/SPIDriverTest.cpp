#include <array>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "OsMock.hpp"
#include "efm_support/clock.h"
#include "efm_support/dma.h"
#include "mcu/io_map.h"
#include "mock/emlib.h"
#include "spi/efm.h"

using testing::_;
using testing::Test;
using testing::NiceMock;
using testing::Eq;
using testing::DoAll;
using testing::SetArgPointee;
using testing::Return;
using testing::InSequence;
using testing::ReturnArg;

using drivers::spi::EFMSPIInterface;
namespace
{
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

        NiceMock<OSMock> _os;
        OSReset _osReset;
    };

    SPIDriverTest::SPIDriverTest()
    {
        this->_cmuReset = InstallProxy(&this->_cmu);
        this->_dmaReset = InstallProxy(&this->_dma);
        this->_usartReset = InstallProxy(&this->_usart);
        this->_osReset = InstallProxy(&this->_os);

        ON_CALL(this->_os, EventGroupWaitForBits(_, _, _, _, _)).WillByDefault(ReturnArg<1>());
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

            EXPECT_CALL(this->_usart, Command(io_map::SPI::Peripheral, USART_CMD_CLEARRX));
            EXPECT_CALL(this->_usart, Command(io_map::SPI::Peripheral, USART_CMD_CLEARTX));
            EXPECT_CALL(this->_usart, IntClear(io_map::SPI::Peripheral, _));

            EXPECT_CALL(this->_dma, PeripheralMemory(1, rxSignal, _, _, true, 10, dmadrvDataSize1, _, _));
            EXPECT_CALL(this->_dma, MemoryPeripheral(2, txSignal, _, _, true, 10, dmadrvDataSize1, _, _));
        }

        EFMSPIInterface spi;

        spi.Initialize();

        std::array<uint8_t, 10> buf;

        spi.Read(buf);
    }

    TEST_F(SPIDriverTest, ShouldWriteProperly)
    {
        auto txSignal = efm::DMASignal<efm::DMASignalUSART::TXBL>(io_map::SPI::Peripheral);

        {
            InSequence s;

            EXPECT_CALL(this->_dma, AllocateChannel(_, nullptr)).WillOnce(DoAll(SetArgPointee<0>(1), Return(ECODE_EMDRV_DMADRV_OK)));
            EXPECT_CALL(this->_dma, AllocateChannel(_, nullptr)).WillOnce(DoAll(SetArgPointee<0>(2), Return(ECODE_EMDRV_DMADRV_OK)));

            EXPECT_CALL(this->_usart, Command(io_map::SPI::Peripheral, USART_CMD_RXBLOCKEN));

            EXPECT_CALL(this->_usart, Command(io_map::SPI::Peripheral, USART_CMD_CLEARRX));
            EXPECT_CALL(this->_usart, Command(io_map::SPI::Peripheral, USART_CMD_CLEARTX));
            EXPECT_CALL(this->_usart, IntClear(io_map::SPI::Peripheral, _));

            EXPECT_CALL(this->_dma, MemoryPeripheral(2, txSignal, _, _, true, 10, dmadrvDataSize1, _, _));

            EXPECT_CALL(this->_usart, IntClear(io_map::SPI::Peripheral, _));

            EXPECT_CALL(this->_usart, Command(io_map::SPI::Peripheral, USART_CMD_RXBLOCKDIS));
        }

        EFMSPIInterface spi;

        spi.Initialize();

        std::array<uint8_t, 10> buf;

        spi.Write(buf);
    }

    TEST_F(SPIDriverTest, ShouldWritePropertlyMoreThan1024Bytes)
    {
        auto txSignal = efm::DMASignal<efm::DMASignalUSART::TXBL>(io_map::SPI::Peripheral);

        auto buf = new uint8_t[2148];
        {
            InSequence s;

            EXPECT_CALL(this->_dma, AllocateChannel(_, nullptr)).WillOnce(DoAll(SetArgPointee<0>(1), Return(ECODE_EMDRV_DMADRV_OK)));
            EXPECT_CALL(this->_dma, AllocateChannel(_, nullptr)).WillOnce(DoAll(SetArgPointee<0>(2), Return(ECODE_EMDRV_DMADRV_OK)));

            EXPECT_CALL(this->_usart, Command(io_map::SPI::Peripheral, USART_CMD_RXBLOCKEN));

            EXPECT_CALL(this->_usart, Command(io_map::SPI::Peripheral, USART_CMD_CLEARRX));
            EXPECT_CALL(this->_usart, Command(io_map::SPI::Peripheral, USART_CMD_CLEARTX));
            EXPECT_CALL(this->_usart, IntClear(io_map::SPI::Peripheral, _));

            EXPECT_CALL(this->_dma, MemoryPeripheral(2, txSignal, _, buf, true, 1024, dmadrvDataSize1, _, _));

            EXPECT_CALL(this->_usart, IntClear(io_map::SPI::Peripheral, _));

            EXPECT_CALL(this->_usart, Command(io_map::SPI::Peripheral, USART_CMD_CLEARRX));
            EXPECT_CALL(this->_usart, Command(io_map::SPI::Peripheral, USART_CMD_CLEARTX));
            EXPECT_CALL(this->_usart, IntClear(io_map::SPI::Peripheral, _));

            EXPECT_CALL(this->_dma, MemoryPeripheral(2, txSignal, _, buf + 1024, true, 1024, dmadrvDataSize1, _, _));

            EXPECT_CALL(this->_usart, IntClear(io_map::SPI::Peripheral, _));

            EXPECT_CALL(this->_usart, Command(io_map::SPI::Peripheral, USART_CMD_CLEARRX));
            EXPECT_CALL(this->_usart, Command(io_map::SPI::Peripheral, USART_CMD_CLEARTX));
            EXPECT_CALL(this->_usart, IntClear(io_map::SPI::Peripheral, _));

            EXPECT_CALL(this->_dma, MemoryPeripheral(2, txSignal, _, buf + 2048, true, 100, dmadrvDataSize1, _, _));
            EXPECT_CALL(this->_usart, IntClear(io_map::SPI::Peripheral, _));

            EXPECT_CALL(this->_usart, Command(io_map::SPI::Peripheral, USART_CMD_RXBLOCKDIS));
        }

        EFMSPIInterface spi;

        spi.Initialize();

        spi.Write(gsl::make_span(buf, 2148));

        delete[] buf;
    }

    TEST_F(SPIDriverTest, ShouldReadProperlyMoreThan1024Bytes)
    {
        auto rxSignal = efm::DMASignal<efm::DMASignalUSART::RXDATAV>(io_map::SPI::Peripheral);
        auto txSignal = efm::DMASignal<efm::DMASignalUSART::TXBL>(io_map::SPI::Peripheral);

        auto buf = new uint8_t[2148];

        {
            InSequence s;

            EXPECT_CALL(this->_dma, AllocateChannel(_, nullptr)).WillOnce(DoAll(SetArgPointee<0>(1), Return(ECODE_EMDRV_DMADRV_OK)));
            EXPECT_CALL(this->_dma, AllocateChannel(_, nullptr)).WillOnce(DoAll(SetArgPointee<0>(2), Return(ECODE_EMDRV_DMADRV_OK)));

            EXPECT_CALL(this->_usart, Command(io_map::SPI::Peripheral, USART_CMD_CLEARRX));
            EXPECT_CALL(this->_usart, Command(io_map::SPI::Peripheral, USART_CMD_CLEARTX));
            EXPECT_CALL(this->_usart, IntClear(io_map::SPI::Peripheral, _));

            EXPECT_CALL(this->_dma, PeripheralMemory(1, rxSignal, buf, _, true, 1024, dmadrvDataSize1, _, _));
            EXPECT_CALL(this->_dma, MemoryPeripheral(2, txSignal, _, _, true, 1024, dmadrvDataSize1, _, _));

            EXPECT_CALL(this->_usart, Command(io_map::SPI::Peripheral, USART_CMD_CLEARRX));
            EXPECT_CALL(this->_usart, Command(io_map::SPI::Peripheral, USART_CMD_CLEARTX));
            EXPECT_CALL(this->_usart, IntClear(io_map::SPI::Peripheral, _));

            EXPECT_CALL(this->_dma, PeripheralMemory(1, rxSignal, buf + 1024, _, true, 1024, dmadrvDataSize1, _, _));
            EXPECT_CALL(this->_dma, MemoryPeripheral(2, txSignal, _, _, true, 1024, dmadrvDataSize1, _, _));

            EXPECT_CALL(this->_usart, Command(io_map::SPI::Peripheral, USART_CMD_CLEARRX));
            EXPECT_CALL(this->_usart, Command(io_map::SPI::Peripheral, USART_CMD_CLEARTX));
            EXPECT_CALL(this->_usart, IntClear(io_map::SPI::Peripheral, _));

            EXPECT_CALL(this->_dma, PeripheralMemory(1, rxSignal, buf + 2048, _, true, 100, dmadrvDataSize1, _, _));
            EXPECT_CALL(this->_dma, MemoryPeripheral(2, txSignal, _, _, true, 100, dmadrvDataSize1, _, _));
        }

        EFMSPIInterface spi;

        spi.Initialize();

        spi.Read(gsl::make_span(buf, 2148));
    }

    TEST_F(SPIDriverTest, ShouldReturnTimeoutOnDMATimeoutWhenReading)
    {
        auto rxSignal = efm::DMASignal<efm::DMASignalUSART::RXDATAV>(io_map::SPI::Peripheral);
        auto txSignal = efm::DMASignal<efm::DMASignalUSART::TXBL>(io_map::SPI::Peripheral);

        auto buf = new uint8_t[2148];

        {
            InSequence s;

            EXPECT_CALL(this->_dma, AllocateChannel(_, nullptr)).WillOnce(DoAll(SetArgPointee<0>(1), Return(ECODE_EMDRV_DMADRV_OK)));
            EXPECT_CALL(this->_dma, AllocateChannel(_, nullptr)).WillOnce(DoAll(SetArgPointee<0>(2), Return(ECODE_EMDRV_DMADRV_OK)));

            EXPECT_CALL(this->_usart, Command(io_map::SPI::Peripheral, USART_CMD_CLEARRX));
            EXPECT_CALL(this->_usart, Command(io_map::SPI::Peripheral, USART_CMD_CLEARTX));
            EXPECT_CALL(this->_usart, IntClear(io_map::SPI::Peripheral, _));

            EXPECT_CALL(this->_dma, PeripheralMemory(1, rxSignal, buf, _, true, 1024, dmadrvDataSize1, _, _));
            EXPECT_CALL(this->_dma, MemoryPeripheral(2, txSignal, _, _, true, 1024, dmadrvDataSize1, _, _));

            EXPECT_CALL(this->_os, EventGroupWaitForBits(_, _, _, _, _)).WillOnce(Return(0xFF));

            EXPECT_CALL(this->_usart, Command(io_map::SPI::Peripheral, USART_CMD_CLEARRX));
            EXPECT_CALL(this->_usart, Command(io_map::SPI::Peripheral, USART_CMD_CLEARTX));
            EXPECT_CALL(this->_usart, IntClear(io_map::SPI::Peripheral, _));

            EXPECT_CALL(this->_dma, PeripheralMemory(1, rxSignal, buf + 1024, _, true, 1024, dmadrvDataSize1, _, _));
            EXPECT_CALL(this->_dma, MemoryPeripheral(2, txSignal, _, _, true, 1024, dmadrvDataSize1, _, _));

            EXPECT_CALL(this->_os, EventGroupWaitForBits(_, _, _, _, _)).WillOnce(Return(0));

            //            EXPECT_CALL(this->_usart, Command(io_map::SPI::Peripheral, USART_CMD_CLEARRX));
            //            EXPECT_CALL(this->_usart, Command(io_map::SPI::Peripheral, USART_CMD_CLEARTX));
            //            EXPECT_CALL(this->_usart, IntClear(io_map::SPI::Peripheral, _));
            //
            //            EXPECT_CALL(this->_dma, PeripheralMemory(1, rxSignal, buf + 2048, _, true, 100, dmadrvDataSize1, _, _));
            //            EXPECT_CALL(this->_dma, MemoryPeripheral(2, txSignal, _, _, true, 100, dmadrvDataSize1, _, _));
        }

        EFMSPIInterface spi;

        spi.Initialize();

        auto r = spi.Read(gsl::make_span(buf, 2148));

        ASSERT_THAT(r, Eq(OSResult::Timeout));
    }
}
